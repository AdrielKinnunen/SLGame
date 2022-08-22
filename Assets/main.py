from PIL import Image
from collections import defaultdict
import json
import os

colors = {
    (0,0,0): "0",
    (255,0,0): '1', 
    (0,255,0): '2', 
    (255,255,0): '3', 
    (0,0,255): '4', 
    (255,0,255): '5', 
    (0,255,255): '6', 
    (255,255,255): '7', 
}

def parsePixel(i: tuple, byteArr):
    byteArr.append(colors[i])

outputJson = []

for root, dirs, files in os.walk("images"):
    for file in files:
        if file.endswith(".bmp"):
            imagePath = os.path.join(root, file)

            im = Image.open(imagePath).convert('RGB')
            print(f"Found {file}")
            print(im.format, im.size, im.mode)

            byteArr = []

            for i in range(im.width):
                for j in range(im.height):
                    pixel = im.getpixel((j,i))
                    parsePixel(pixel, byteArr)
                    

            output = defaultdict()
            output['name'] = file
            output['width']  = im.width
            output['height']  = im.height
            output['data'] = byteArr
            outputJson.append(output)

with open('output.json', 'w', encoding='utf-8') as f:
    json.dump(outputJson, f, ensure_ascii=False, indent=4)