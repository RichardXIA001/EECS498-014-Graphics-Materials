# Report

**Notes: all of the images, revised codes and logs are contained in corresponding directories.**

## GAN

1. Original

**Task 2 for GAN**: Difference between Eq2 and Eq1: 

Eq2 is trying to minize the score of fake images and maximize that of real images, while in Eq1 the generator is trying to minize the scores for fake images.

Becasue the generator always wanted to generate more plausible images and confuse the discriminator, but the discriminator always wanted to select those real images and give penalty to fake images

2. Add R1

3. Add augmentation


## Diffusion

**Task 3**

1. Time cost for DDPM/DDIM to generate 10 images:

     * DDPM(1000 Steps): 342 seconds

     * DDIM(50 Steps): 17 seconds

     * Pros of using DDIM: Much faster for inference  

     * Cons of using DDPM: Loss of Stochasticity, Limited Robustness to Noise

2. DPS:

DPS takes slightly longer than DDIM, especially at higher timesteps, due to its non-linear scaling.

Reducing timesteps results in a quality drop for both DPS and DDIM, but DPS retains slightly better quality than DDIM at lower timestep counts.

