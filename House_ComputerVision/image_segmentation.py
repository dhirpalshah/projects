from transformers import AutoImageProcessor, SegformerForSemanticSegmentation
import tensorflow as tf
from PIL import Image
import requests
import matplotlib.pyplot as plt
import numpy as np
from palette import ade_palette

def preprocess(image):
    image_processor = AutoImageProcessor.from_pretrained("nvidia/segformer-b0-finetuned-ade-512-512")
    model = SegformerForSemanticSegmentation.from_pretrained("nvidia/segformer-b0-finetuned-ade-512-512")

    # image = Image.open(r"images/2.jpeg")

    inputs = image_processor(images=image, return_tensors="pt")
    outputs = model(**inputs)
    logits = outputs.logits  # shape (batch_size, num_labels, height/4, width/4)
    list(logits.shape)
    return logits

def segment(logits, image):
    logits = tf.transpose(logits.detach(), [0, 2, 3, 1])

    upsampled_logits = tf.image.resize(
        logits,
        # We reverse the shape of `image` because `image.size` returns width and height.
        image.size[::-1],
    )

    pred_seg = tf.math.argmax(upsampled_logits, axis=-1)[0]

    return pred_seg

def plot(segmentation_map, image, image_num):
    color_seg = np.zeros((segmentation_map.shape[0], segmentation_map.shape[1], 3), dtype=np.uint8)
    palette = np.array(ade_palette)
    for label, color in enumerate(palette):
        color_seg[segmentation_map == label, :] = color
    color_seg = color_seg[..., ::-1]  # convert to BGR

    img = np.array(image) * 0.5 + color_seg * 0.5  # plot the image with the segmentation map
    img = img.astype(np.uint8)

    plt.figure(figsize=(15, 10))
    plt.imsave(f'{image_num}test.jpeg', img)

def generate_images():
    for i in range(1,11):
        path = f"images/{i}.jpeg"
        image = Image.open(path)
        logits = preprocess(image)
        segmap = segment(logits, image)
        plot(segmap, image, i)


if __name__ == '__main__':
    generate_images()


