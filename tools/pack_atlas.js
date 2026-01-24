const fs = require('fs');
const path = require('path');
const texturePacker = require('free-tex-packer-core');

const inputDir = process.argv[2];
const outputDir = process.argv[3];
const templateFile = process.argv[4];

if (!inputDir || !outputDir || !templateFile) {
    console.error("Usage: node pack_atlas.js <inputDir> <outputDir> <templateFile>");
    process.exit(1);
}

function getFiles(dir, fileList = [], baseDir = dir) {
    if (!fs.existsSync(dir)) return [];
    
    const files = fs.readdirSync(dir);
    files.forEach(file => {
        const filePath = path.join(dir, file);
        const stat = fs.statSync(filePath);
        if (stat.isDirectory()) {
            getFiles(filePath, fileList, baseDir);
        } else {
            if (path.extname(file).toLowerCase() === '.png') {
                const relativePath = path.relative(baseDir, filePath).replace(/\\/g, '/');
                // free-tex-packer-core expects 'path' or 'name' usually.
                fileList.push({
                    path: relativePath, 
                    contents: fs.readFileSync(filePath)
                });
            }
        }
    });
    return fileList;
}

const images = getFiles(inputDir);
console.log(`Packing ${images.length} images from ${inputDir} to ${outputDir}...`);

const packerOptions = {
    textureName: "atlas",
    width: 4096,
    height: 4096,
    padding: 4,
    allowRotation: false,
    detectIdentical: true,
    allowTrim: true,
    trimMode:'trim',
    alphaThreshold:0,
    powerOfTwo: true,
    fixedSize: true,
    packer: "MaxRectsBin",
    exporter: {
        template: templateFile,
        fileExt: "h"
    },
    removeFileExtension: true,
    prependFolderName: true
};

texturePacker(images, packerOptions, (files, error) => {
    if (error) {
        console.error('Packing failed:', error);
        process.exit(1);
    }
    
    for (const item of files) {
        const outPath = path.join(outputDir, item.name);
        fs.writeFileSync(outPath, item.buffer);
        console.log(`Written ${outPath}`);
    }
});
