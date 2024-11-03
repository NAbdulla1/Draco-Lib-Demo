const fs = require('fs');
const express = require('express');
const draco3d = require('draco3d'); // Import the draco3d library

const app = express();
const PORT = 3000;

// Function to find an attribute by metadata name and value
async function findAttributeByMetadata(decoder, mesh, metadataQuerier, name, value) {
    const numAttributes = mesh.num_attributes();
    for (let i = 0; i < numAttributes; i++) {
        const attributeMetadata = decoder.GetAttributeMetadata(mesh, i);
        
        if (attributeMetadata) {
            const entryValue = metadataQuerier.GetStringEntry(attributeMetadata, name);
            if (entryValue === value) {
                return i;  // Return the attribute ID if we find a match
            }
        }
    }
    return -1; // Return -1 if no matching attribute is found
}

async function decodeMesh() {
    // Initialize the Draco decoder
    const dracoDecoderModule = await draco3d.createDecoderModule({});
    const buffer = new dracoDecoderModule.DecoderBuffer();
    const decoder = new dracoDecoderModule.Decoder();
    const metadataQuerier = new dracoDecoderModule.MetadataQuerier();

    // Read the compressed Draco file
    const encodedData = fs.readFileSync('../build/compressed_mesh.drc');
    buffer.Init(new Int8Array(encodedData), encodedData.length);

    // Check if the data is a mesh
    const geometryType = decoder.GetEncodedGeometryType(buffer);
    if (geometryType !== dracoDecoderModule.TRIANGULAR_MESH) {
        throw new Error("The file does not contain a mesh.");
    }

    // Decode the buffer into a Draco mesh
    const mesh = new dracoDecoderModule.Mesh();
    const status = decoder.DecodeBufferToMesh(buffer, mesh);
    if (!status.ok()) {
        throw new Error("Decoding failed: " + status.error_msg());
    }

    // Extract position attribute data
    const { posAttributeData, positions } = decodePositions();

    const { uvAttributeData, uvs } = decodeUVs();
    const { uvAttributeData2, uvs2 } = await decodeUV2s();

    // Clean up Draco resources
    dracoDecoderModule.destroy(posAttributeData);
    dracoDecoderModule.destroy(uvAttributeData);
    dracoDecoderModule.destroy(uvAttributeData2);
    dracoDecoderModule.destroy(mesh);
    dracoDecoderModule.destroy(decoder);
    dracoDecoderModule.destroy(buffer);

    return {positions, uvs, uvs2};

    function decodePositions() {
        const positionAttrId = decoder.GetAttributeId(mesh, dracoDecoderModule.POSITION);
        if (positionAttrId === -1) {
            throw new Error("No position attribute found.");
        }
        const positionAttribute = decoder.GetAttribute(mesh, positionAttrId);

        // Retrieve position data
        const numPoints = mesh.num_points();
        const posAttributeData = new dracoDecoderModule.DracoFloat32Array();
        decoder.GetAttributeFloatForAllPoints(mesh, positionAttribute, posAttributeData);

        const positions = [];
        for (let i = 0; i < numPoints; i++) {
            positions.push({
                x: posAttributeData.GetValue(i * 3),
                y: posAttributeData.GetValue(i * 3 + 1),
                z: posAttributeData.GetValue(i * 3 + 2)
            });
        }
        return { posAttributeData, positions };
    }

    function decodeUVs() {
        const uvAttrId = decoder.GetAttributeId(mesh, dracoDecoderModule.TEX_COORD);
        if (uvAttrId === -1) {
            throw new Error("No uv attribute found.");
        }
        const uvAttribute = decoder.GetAttribute(mesh, uvAttrId);

        // Retrieve position data
        const numPoints = mesh.num_points();
        const uvAttributeData = new dracoDecoderModule.DracoFloat32Array();
        decoder.GetAttributeFloatForAllPoints(mesh, uvAttribute, uvAttributeData);

        const uvs = [];
        for (let i = 0; i < numPoints; i++) {
            uvs.push({
                x: uvAttributeData.GetValue(i * 2),
                y: uvAttributeData.GetValue(i * 2 + 1)
            });
        }
        return { uvAttributeData, uvs };
    }

    async function decodeUV2s() {
        //const uvAttrId = await findAttributeByMetadata(decoder, mesh, metadataQuerier, 'name', 'secondaryUvs'); // also works
        const uvAttrId = decoder.GetAttributeIdByMetadataEntry(mesh, 'name', 'secondaryUvs');
        if (uvAttrId === -1) {
            throw new Error("No uv2 attribute found.");
        }
        const uvAttribute = decoder.GetAttribute(mesh, uvAttrId);

        // Retrieve position data
        const numPoints = mesh.num_points();
        const uvAttributeData2 = new dracoDecoderModule.DracoFloat32Array();
        decoder.GetAttributeFloatForAllPoints(mesh, uvAttribute, uvAttributeData2);

        const uvs2 = [];
        for (let i = 0; i < numPoints; i++) {
            uvs2.push({
                x: uvAttributeData2.GetValue(i * 2),
                y: uvAttributeData2.GetValue(i * 2 + 1)
            });
        }
        return { uvAttributeData2, uvs2 };
    }
}

// Set up the root route to decode and respond with mesh data
app.get('/', async (req, res) => {
    try {
        const geoData = await decodeMesh();
        res.json({ geoData });
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// Start the server
app.listen(PORT, () => {
    console.log(`Server running on http://localhost:${PORT}`);
});
