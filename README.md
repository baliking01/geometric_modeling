# geometric_modeling
University course for geometric modeling
This repository contains some of the implementation of certain algorithms from `https://web.uni-miskolc.hu/~matip/geometric/`, which have been covered during the lectures.

## Semester assignment (Bézier Surface)
The folder `bezier_surface` contains the related project, which is a modified version of the `me-courses\grafika\examples\cube` from a previous graphics course. The modified program generates an **n** by **m** grid control mesh with some **r** resolution. The former two parameters determine the final mesh's size, while the resolution is responsible for surface smoothness. 

### Surface calculations
The algorithm used to calculate the interpolated values employs the help of Bernstein polynomials and is described the following way.

```math
\textbf{s}(u, v) = \sum_{i=0}^n \sum_{j=0}^m \textbf{b}_{ij}B_i^n(u)B_j^m(v).
```
where the parameters **u** and **v** are constrained such that $(u,v) \in [0, 1]^2$.

Each Bernstein polynomial is calculated according to the formula:
```math
B_i^n(t) = \binom{n}{i}t^i(1-t)^{n-i}, i = 1, 2, ..., n.
```
### Factorial optimizations
For faster calculation of the binomial coeffiecient, a factorial lookup table is implemented alongside a relatively fast algorithm for dealing with cases, where the factorial itself is too big to be stored in any type of variable. The surface calculations rely on the lookup table and when a binomial coeffeicient containing in its expansion a number greater than 27! is required, the algorithm is used to find the requested value.

### Surface oscillation
To make the scene seem dynamic and to demonstrate the interesting visual effects of the surface it is moved up and down in a non-uniform manner. In order to achieve this, the following solution was used:
  - Upon starting the program the control points are generated at random **z** values
  - Each control point has, in a separate array, its own **dz** displacement value, which is partially incremented by a random number
  - The **dz** value is used as input to a modified sine wave that oscillates between 0 and 1: $(sin(dz) + 1) / 2$, which is scaled by 4 to make the change more visible
  - Output from the function is then used as the new $$z$$ value for the given control point

### Surface normals
Every control point is either located on the corners, edges or in the middle of the surface. With these three conditions, by considering the neighbours of a given control point, we can approximate its two tangent lines and by taking their cross product we get the normal of the control point.

### Texture and lighting
Finally, to complete the visual effect, a texture is mapped onto the surface and since we previously calculated the surface normals, a light can also be added. Mapping the texture onto the surface is surprisingly easy, since the domain of the entire surface $\textbf{s}(u,v)$ is $(u, v) \in [0, 1]^2$ and conviniently the texture coordinates are also in $[0, 1]^2$, so they correspond perfectly to each other.

### User interactions
The user can change the camera position with the familiar `w, a, s, d` keys and also rotate the camera by holding down the `left mouse button`. Up and down movement is done by holding the `space` and `LCTRL` keys, respectively. By default, surface normals and the control polygon are hidden. These can be toggled by pressing the `n` key for normals and the `p` key for the polygon. Furthermore, the dimensions of the surface can also be varied via the arrow keys. The `up` and `down` keys increase or decrease the **n** dimensional variable, and similarly the `left` and `right` arrow keys change the **m** dimensional variable. Finally, the user can toggle the texture on or off with the `t` key, either showing the default texture or the individual primitive quads that make up the surface.
