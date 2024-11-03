#include <draco/compression/encode.h>
#include <draco/compression/decode.h>
#include <draco/core/data_buffer.h>
#include <draco/mesh/mesh.h>
#include <draco/attributes/geometry_attribute.h>
#include <iostream>
#include <memory>
#include <fstream>
#include "main.h"

int encodeAndSave() {
        // Create an empty mesh
    draco::Mesh mesh;

    int num_vertices = 3;

    // Allocate space for vertices
    mesh.set_num_points(num_vertices);
    float vertex_data[3][3] = {
        {0.0f, 0.0f, 0.0f}, // Vertex 0
        {1.0f, 0.0f, 0.0f}, // Vertex 1
        {0.0f, 1.0f, 0.0f}  // Vertex 2
    };

    float uv1s[3][3] = {
        {0.0f, 0.0f},
        {0.5f, 0.5f},
        {1.0f, 1.0f}};

    float uv2s[3][3] = {
        {0.1f, 0.1f},
        {0.6f, 0.6f},
        {0.9f, 0.9f}};

    // Add a position attribute for vertices (3D coordinates)
    draco::GeometryAttribute position_attribute;
    position_attribute.Init(draco::GeometryAttribute::POSITION, nullptr, 3, draco::DT_FLOAT32, false, sizeof(float) * 3, 0);
    const int pos_att_id = mesh.AddAttribute(position_attribute, true, num_vertices);

    draco::GeometryAttribute uv_attr1;
    uv_attr1.Init(draco::GeometryAttribute::TEX_COORD, nullptr, 2, draco::DT_FLOAT32, false, sizeof(float) * 2, 0);
    const int uv_attr1_id = mesh.AddAttribute(uv_attr1, true, num_vertices);

    draco::GeometryAttribute uv_attr2;
    uv_attr2.Init(draco::GeometryAttribute::TEX_COORD, nullptr, 2, draco::DT_FLOAT32, false, sizeof(float) * 2, 0);
    const int uv_attr2_id = mesh.AddAttribute(uv_attr2, true, num_vertices);

    for (int i = 0; i < num_vertices; i++)
    {
        float uv1[2], uv2[2];
        float vv[3];
        for (int j = 0; j < 3; j++)
        {
            vv[j] = vertex_data[i][j];
        }
        for (int j = 0; j < 2; j++)
        {
            uv1[j] = uv1s[i][j];
            uv2[j] = uv2s[i][j];
        }
        mesh.attribute(pos_att_id)->SetAttributeValue(draco::AttributeValueIndex(i), vv);
        mesh.attribute(uv_attr1_id)->SetAttributeValue(draco::AttributeValueIndex(i), uv1);
        mesh.attribute(uv_attr2_id)->SetAttributeValue(draco::AttributeValueIndex(i), uv2);
    }

    // Create metadata for the position attribute
    auto uv2_metadata = std::make_unique<draco::AttributeMetadata>();
    uv2_metadata->AddEntryString("name", "secondaryUvs");
    uv2_metadata->AddEntryInt("custom_id", 44); // we can't use this to retrieve attribute id
    mesh.AddAttributeMetadata(uv_attr2_id, std::move(uv2_metadata));

    // Define a single face with the three vertices
    mesh.SetNumFaces(1);
    draco::Mesh::Face face = {draco::PointIndex(0), draco::PointIndex(1), draco::PointIndex(2)};
    mesh.SetFace(draco::FaceIndex(0), face);

    std::cout << "Going to compress..." << std::endl;

    // Compress the mesh
    draco::Encoder encoder;
    draco::EncoderBuffer buffer;
    const draco::Status status = encoder.EncodeMeshToBuffer(mesh, &buffer);

    if (!status.ok())
    {
        std::cerr << "Error encoding the mesh: " << status.error_msg_string() << std::endl;
        return -1;
    }

    // Save the encoded data to a file
    std::ofstream out_file("compressed_mesh.drc", std::ios::binary);
    if (!out_file) {
        std::cerr << "Failed to open file for writing." << std::endl;
        return -1;
    }
    out_file.write(buffer.data(), buffer.size());
    out_file.close();

    std::cout << "Compressed mesh size: " << buffer.size() << " bytes, saved in file too." << std::endl;
    return 0;
}

int main()
{
    encodeAndSave();

    bool retFlag;
    int retVal = decodeFileAndPrint(retFlag);
    if (retFlag)
        return retVal;

    return 0;
}

int decodeFileAndPrint(bool &retFlag)
{
    retFlag = true;
    // Read the compressed data from file
    std::ifstream in_file("compressed_mesh.drc", std::ios::binary | std::ios::ate);
    if (!in_file)
    {
        std::cerr << "Failed to open file for reading." << std::endl;
        return -1;
    }

    std::streamsize file_size = in_file.tellg();
    in_file.seekg(0, std::ios::beg);
    std::vector<char> buffer(file_size);
    if (!in_file.read(buffer.data(), file_size))
    {
        std::cerr << "Failed to read file content." << std::endl;
        return -1;
    }
    in_file.close();

    // Decode the mesh
    draco::Decoder decoder;
    draco::DecoderBuffer decoderBuffer;
    decoderBuffer.Init(buffer.data(), buffer.size());
    auto decodedMesh = decoder.DecodeMeshFromBuffer(&decoderBuffer);

    if (!decodedMesh.ok())
    {
        std::cerr << "Error decoding the mesh: " << decodedMesh.status().error_msg_string() << std::endl;
        return -1;
    }

    std::cout << "Successfully decompressed the mesh!" << std::endl;

    // Access the decompressed mesh's position attribute
    const draco::Mesh *mesh_ptr = decodedMesh.value().get();
    const draco::PointAttribute *pos_attr = mesh_ptr->GetNamedAttribute(draco::GeometryAttribute::POSITION);

    if (pos_attr)
    {
        // Retrieve and print the position data of each vertex
        draco::Vector3f pos; // Holds the position data (x, y, z)
        for (draco::PointIndex i(0); i < mesh_ptr->num_points(); ++i)
        {
            if (pos_attr->ConvertValue<float, 3>(pos_attr->mapped_index(i), &pos[0]))
            {
                std::cout << "Vertex " << i.value() << ": (" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")\n";
            }
            else
            {
                std::cerr << "Error retrieving position for vertex " << i.value() << std::endl;
            }
        }
    }
    else
    {
        std::cerr << "Position attribute not found!" << std::endl;
    }

    const draco::PointAttribute *uv1_attr_ret = mesh_ptr->GetNamedAttribute(draco::GeometryAttribute::TEX_COORD);
    if (uv1_attr_ret)
    {
        draco::Vector2f uv1;
        for (draco::PointIndex i(0); i < mesh_ptr->num_points(); i++)
        {
            if (uv1_attr_ret->ConvertValue<float, 2>(uv1_attr_ret->mapped_index(i), &uv1[0]))
            {
                std::cout << "UV " << i.value() << ": (" << uv1[0] << ", " << uv1[1] << ")" << std::endl;
            }
        }
    }
    else
    {
        std::cerr << "uv1 attribute not found!" << std::endl;
    }

    //    const draco::PointAttribute *uv2_attr_ret = mesh_ptr->GetNamedAttributeByUniqueId(draco::GeometryAttribute::TEX_COORD, 2);//works
    //    const draco::PointAttribute *uv2_attr_ret = mesh_ptr->GetAttributeByUniqueId(2);//works
    const std::string metaName = "name";
    const std::string metaValue = "secondaryUvs";
    const int uv2_attr_id = mesh_ptr->GetAttributeIdByMetadataEntry(metaName, metaValue);
    if (uv2_attr_id == -1)
    {
        std::cerr << "uv2 can't found by metadata entry" << std::endl;
        return -1;
    }
    const draco::PointAttribute *uv2_attr_ret = mesh_ptr->GetAttributeByUniqueId(uv2_attr_id);
    if (uv2_attr_ret)
    {
        draco::Vector2f uv2;
        for (draco::PointIndex i(0); i < mesh_ptr->num_points(); i++)
        {
            if (uv2_attr_ret->ConvertValue<float, 2>(uv2_attr_ret->mapped_index(i), &uv2[0]))
            {
                std::cout << "UV2 " << i.value() << ": (" << uv2[0] << ", " << uv2[1] << ")" << std::endl;
            }
        }
    }
    else
    {
        std::cerr << "uv2 attribute not found!" << std::endl;
    }
    retFlag = false;
    return {};
}
