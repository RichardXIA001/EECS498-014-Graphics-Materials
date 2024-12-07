# Differentiable Augmentation for Data-Efficient GAN Training
# Shengyu Zhao, Zhijian Liu, Ji Lin, Jun-Yan Zhu, and Song Han
# https://arxiv.org/pdf/2006.10738

import numpy as np
import cv2
import torch
import torch.nn.functional as F


def DiffAugment(x, policy='', channels_first=True):
    if policy:
        if not channels_first:
            x = x.permute(0, 3, 1, 2)
        for p in policy.split(','):
            for f in AUGMENT_FNS[p]:
                x = f(x)
        if not channels_first:
            x = x.permute(0, 2, 3, 1)
        x = x.contiguous()
    return x


def rand_brightness(x):
    x = x + (torch.rand(x.size(0), 1, 1, 1, dtype=x.dtype, device=x.device) - 0.5)
    return x


def rand_saturation(x):
    x_mean = x.mean(dim=1, keepdim=True)
    x = (x - x_mean) * (torch.rand(x.size(0), 1, 1, 1, dtype=x.dtype, device=x.device) * 2) + x_mean
    return x


def rand_contrast(x):
    x_mean = x.mean(dim=[1, 2, 3], keepdim=True)
    x = (x - x_mean) * (torch.rand(x.size(0), 1, 1, 1, dtype=x.dtype, device=x.device) + 0.5) + x_mean
    return x


def rand_translation(x, ratio=0.125):
    shift_x, shift_y = int(x.size(2) * ratio + 0.5), int(x.size(3) * ratio + 0.5)
    translation_x = torch.randint(-shift_x, shift_x + 1, size=[x.size(0), 1, 1], device=x.device)
    translation_y = torch.randint(-shift_y, shift_y + 1, size=[x.size(0), 1, 1], device=x.device)
    grid_batch, grid_x, grid_y = torch.meshgrid(
        torch.arange(x.size(0), dtype=torch.long, device=x.device),
        torch.arange(x.size(2), dtype=torch.long, device=x.device),
        torch.arange(x.size(3), dtype=torch.long, device=x.device),
    )
    grid_x = torch.clamp(grid_x + translation_x + 1, 0, x.size(2) + 1)
    grid_y = torch.clamp(grid_y + translation_y + 1, 0, x.size(3) + 1)
    x_pad = F.pad(x, [1, 1, 1, 1, 0, 0, 0, 0])
    x = x_pad.permute(0, 2, 3, 1).contiguous()[grid_batch, grid_x, grid_y].permute(0, 3, 1, 2).contiguous()
    return x


def rand_cutout(x, ratio=0.5):
    ''' 
        Random Cutout Data Augmentation 
    Args:
        x (torch):      Pytorch image with shape (B, C, H, W)     
        ratio (float):  The cut ratio (usually 0.5)
    Returns:
        x (torch):      Pytorch image with cutout 
    '''
    # TODO: Implement the Rand Cutout; The range should be randomly chosen in 25%-50% of the height and width; position is random.

    # Get the size of the image
    B, C, H, W = x.size()

    # Get the cutout size
    cutout_H = int(H * ratio)
    cutout_W = int(W * ratio)

    device = x.device
    # Get the cutout position
    cutout_x = torch.randint(0, H - cutout_H + 1, (B,)).to(device)
    cutout_y = torch.randint(0, W - cutout_W + 1, (B,)).to(device)

    

    h_grid = torch.arange(H, device=device).view(1, H, 1).to(device) # shape: (1,H,1)
    w_grid = torch.arange(W, device=device).view(1, 1, W).to(device) # shape: (1,1,W)

    # Create a mask for all images simultaneously:
    # mask[i,h,w] = True if pixel (h,w) lies within the cutout region of image i
    mask = ((h_grid >= cutout_x[:, None, None]) & (h_grid < (cutout_x + cutout_H)[:, None, None]) &
            (w_grid >= cutout_y[:, None, None]) & (w_grid < (cutout_y + cutout_W)[:, None, None]))
    # mask shape: (B,H,W)

    # Expand mask to cover (B,C,H,W)
    mask = mask.unsqueeze(1).expand(B, C, H, W).to(device)

    # Zero out the pixels in the cutout regions
    x[mask] = 0

    # # Cutout the image
    # x[:, :, cutout_x:cutout_x+cutout_H, cutout_y:cutout_y+cutout_W] = 0

    ###############################################################################################################

    return x


AUGMENT_FNS = {
    'color': [rand_brightness, rand_saturation, rand_contrast],
    'translation': [rand_translation],
    'cutout': [rand_cutout],
}



if __name__ == "__main__":
    sample_path = "many_shot_dog/0.png"

    # Read Image Path
    img = np.array(cv2.imread(sample_path))
    img = img.astype(np.float32)[None, ...]

    # Convert img
    img = torch.from_numpy(img.transpose(0, 3, 1, 2))

    # Execute the cutout augmentation
    cutout_img = rand_cutout(img)

    # Convert back
    cutout_img = np.array(np.uint8(cutout_img.numpy().squeeze(0).transpose(1, 2, 0)))
    cv2.imwrite("cutout_img.png", cutout_img)


    print("Finished!")
