# Mandelbrot Set

The complete illustrated project description is available on the [Mandelbrot GitHub Pages website](https://pdbuchan.github.io/mandelbrot/).

## 1. Introduction

The conventional Mandelbrot escape-time algorithm assigns an integer iteration count to every point in the complex plane. Visual representations of the set then assign a color to each point based upon iteration count.

### 1.1. Theory

For a point $c$ located on the complex plane,

$$
c = u + iv,
$$

starting with

$$
z_0 = 0,
$$

we iterate as follows:

$$
z_{n+1} = z_n^2 + c.
$$

- If $\lvert z_n \rvert \to \infty$, then $c$ is outside the Mandelbrot set.
- If $\\{z_n\\}_{n=0}^{\infty}$ is bounded, then $c$ is inside the Mandelbrot set.

### 1.2. Practical Implementation

If the magnitude of $z_n$ eventually exceeds the escape radius, conventionally 2, the point is known not to belong to the Mandelbrot set.

A straightforward renderer assigns a color according to the integer $n$ at which escape occurs. Because $n$ can take only integer values, all pixels having the same escape iteration receive the same color. The visual result consists of discrete color bands (posterization).

In practice:

- If $\lvert z_n \rvert > 2$, then the orbit definitely escapes. Therefore, $c$ is outside the set and we stop iterating.
- If the maximum iteration count is reached while $\lvert z_n \rvert \le 2$, escape has not been demonstrated. For rendering purposes, $c$ is provisionally treated as belonging to the set and, as is common practice, colored black.

The maximum number of iterations is typically on the order of 1000 to 5000. A larger maximum reduces the number of exterior points that are provisionally misclassified as interior. Visually, the edge of the set becomes more sharply defined as the maximum iteration count increases.

Starting with

$$
c = u_0 + iv_0, \qquad z_0 = 0,
$$

we have

$$
z_{n+1} = z_n^2 + c.
$$

Writing

$$
z_n = u_n + iv_n,
$$

gives

$$
z_{n+1} = (u_n + iv_n)^2 + c
$$
$$
= u_n^2 + 2iu_nv_n - v_n^2 + c
$$
$$
= (u_n^2 - v_n^2 + u_0) + i(2u_nv_n + v_0).
$$

So

$$
\boxed{u_{n+1} = u_n^2 - v_n^2 + u_0}
$$

and

$$
\boxed{v_{n+1} = 2u_nv_n + v_0}.
$$

The absolute value of $z$ at iteration $n$ is

$$
\lvert z_n \rvert = \sqrt{u_n^2 + v_n^2}.
$$

We can forego the square-root operation and compare against 4 rather than 2:

- If $u_n^2 + v_n^2 > 4$, then the orbit will become unbounded. Therefore, $c$ is outside the set, we stop iterating, and we assign a color based on $n$.
- If the maximum iteration count is reached with $u_n^2 + v_n^2 \le 4$, escape has not been demonstrated. For rendering purposes, $c$ is provisionally treated as belonging to the set and, as is common practice, colored black.

At this point, the result can be disappointing because the color distribution does not look as smooth as many Mandelbrot images plotted elsewhere. Typically, the color changes are bunched near the edge of the black set while large areas exhibit visible color bands. Compressing or stretching the palette, or selecting different colors, may alter the appearance but cannot eliminate the fundamental banding caused by using only an integer escape count.

### 1.3. Smooth Color Assignment

Smooth coloring replaces the integer escape count by a real-valued quantity usually written

$$
\boxed{\nu = n + 1 - \frac{\ln\left(\ln \lvert z_n \rvert\right)}{\ln 2}}
$$

or equivalently,

$$
\boxed{\nu = n + 1 - \log_2\left(\ln \lvert z_n \rvert\right)}.
$$

Here $z_n$ is the complex value associated with the escape iteration. This quantity is often called the **normalized iteration count**, **continuous iteration count**, **fractional iteration count**, or **smooth iteration count**.

The formula may at first appear somewhat arbitrary, particularly because it contains a logarithm of a logarithm. In fact, both logarithms arise naturally from the quadratic growth of the Mandelbrot iteration after $z_n$ begins escaping.

## 2. Why Integer Iteration Counts Produce Bands

Suppose two nearby points escape as follows. For the first point,

$$
\lvert z_{50} \rvert = 2.01,
$$

while for a neighboring point,

$$
\lvert z_{50} \rvert = 3.8.
$$

With ordinary escape-time coloring, both points are assigned the same value:

$$
n = 50.
$$

Yet they are at substantially different stages of escape. The second orbit has progressed much farther into the escaping regime than the first.

The integer iteration count throws away this information.

What we want is therefore a way of using the magnitude of the escaping value to estimate a **fractional position between iteration levels**.

Instead of obtaining values such as

```text
50, 50, 50, 51, 51,
```

we would like something more like

```text
50.12, 50.34, 50.76, 51.08, 51.41.
```

A continuous color palette can then vary continuously with this number.

## 3. Behaviour of the Mandelbrot Iteration After Escape

The iteration is

$$
z_{n+1} = z_n^2 + c.
$$

Once $\lvert z_n \rvert$ has become sufficiently large, the $z_n^2$ term dominates the comparatively small constant $c$. We can therefore approximate

$$
z_{n+1} \approx z_n^2.
$$

Taking magnitudes,

$$
\lvert z_{n+1} \rvert \approx \lvert z_n \rvert^2.
$$

Let

$$
r_n = \lvert z_n \rvert.
$$

Then

$$
r_{n+1} \approx r_n^2.
$$

This is the key relation from which smooth iteration coloring follows.

## 4. Growth Over Several Iterations

Starting from

$$
r_{n+1} \approx r_n^2,
$$

one additional iteration gives

$$
r_{n+2} \approx r_{n+1}^2 \approx \left(r_n^2\right)^2 = r_n^4.
$$

Another iteration gives

$$
r_{n+3} \approx r_n^8.
$$

Thus, after $m$ further iterations,

$$
\boxed{r_{n+m} \approx r_n^{2^m}}
$$

because each Mandelbrot iteration squares the previous magnitude.

The exponent therefore grows as

$$
1,\ 2,\ 4,\ 8,\ 16,\ldots
$$

or

$$
2^m.
$$

This extremely rapid growth is the reason that **two logarithms** eventually appear in the smooth-iteration formula.

## 5. Taking the First Logarithm

Take the natural logarithm of

$$
r_{n+m} \approx r_n^{2^m}.
$$

Using

$$
\ln\left(a^b\right) = b\ln a,
$$

we obtain

$$
\boxed{\ln r_{n+m} \approx 2^m \ln r_n}.
$$

The repeated squaring has now become ordinary exponential growth. This is the first important simplification.

## 6. Taking the Second Logarithm

Take the logarithm once again:

$$
\ln\left(\ln r_{n+m}\right)
\approx
\ln\left(2^m \ln r_n\right).
$$

Using

$$
\ln(ab) = \ln a + \ln b,
$$

gives

$$
\ln\left(\ln r_{n+m}\right)
\approx
\ln\left(2^m\right) + \ln\left(\ln r_n\right).
$$

Since

$$
\ln\left(2^m\right) = m\ln 2,
$$

we obtain

$$
\boxed{
\ln\left(\ln r_{n+m}\right)
\approx
m\ln 2 + \ln\left(\ln r_n\right)
}
$$

and therefore

$$
\boxed{
\frac{\ln\left(\ln r_{n+m}\right)}{\ln 2}
\approx
m + \frac{\ln\left(\ln r_n\right)}{\ln 2}
}.
$$

Notice what has happened.

Each additional Mandelbrot iteration increases the value of

$$
\frac{\ln\left(\ln \lvert z \rvert\right)}{\ln 2}
$$

by approximately **one**.

Consequently,

$$
n - \frac{\ln\left(\ln \lvert z_n \rvert\right)}{\ln 2}
$$

is approximately unchanged as the escaping $z_n$ is iterated further.

This is the fundamental mathematical observation behind smooth iteration coloring.

## 7. Constructing an Iteration-Independent Quantity

Define

$$
Q_n = n - \frac{\ln\left(\ln \lvert z_n \rvert\right)}{\ln 2}.
$$

Consider the same orbit one iteration later. Since

$$
\lvert z_{n+1} \rvert \approx \lvert z_n \rvert^2,
$$

we have

$$
\ln \lvert z_{n+1} \rvert \approx 2\ln \lvert z_n \rvert.
$$

Then

$$
\ln\left(\ln \lvert z_{n+1} \rvert\right)
\approx
\ln\left(2\ln \lvert z_n \rvert\right).
$$

Therefore,

$$
\ln\left(\ln \lvert z_{n+1} \rvert\right)
\approx
\ln 2 + \ln\left(\ln \lvert z_n \rvert\right).
$$

Dividing by $\ln 2$,

$$
\frac{\ln\left(\ln \lvert z_{n+1} \rvert\right)}{\ln 2}
\approx
1 + \frac{\ln\left(\ln \lvert z_n \rvert\right)}{\ln 2}.
$$

Consequently,

$$
(n+1) - \frac{\ln\left(\ln \lvert z_{n+1} \rvert\right)}{\ln 2}
$$

is approximately

$$
(n+1) - \left[
1 + \frac{\ln\left(\ln \lvert z_n \rvert\right)}{\ln 2}
\right],
$$

which simplifies to

$$
n - \frac{\ln\left(\ln \lvert z_n \rvert\right)}{\ln 2}.
$$

Thus,

$$
\boxed{Q_{n+1} \approx Q_n}
$$

once $z$ is sufficiently far into its escaping regime.

The combination of the integer iteration number and the double logarithm therefore gives us a continuous measure of essentially the same underlying escape state.

## 8. Where Does the "+1" Come From?

The expression derived above naturally suggests

$$
n - \frac{\ln\left(\ln \lvert z_n \rvert\right)}{\ln 2}.
$$

The formula conventionally used for Mandelbrot coloring, however, is

$$
\nu = n + 1 - \frac{\ln\left(\ln \lvert z_n \rvert\right)}{\ln 2}.
$$

The additional 1 is essentially a **choice of normalization and iteration indexing**. It establishes where the nominal integer boundaries of the continuous scale are placed.

To see this more concretely, suppose we regard $m$ as a fractional iteration offset. Our approximate growth law remains meaningful for real $m$:

$$
r_{n+m} \approx r_n^{2^m}.
$$

Take logarithms:

$$
\ln r_{n+m} \approx 2^m \ln r_n.
$$

Choose a fixed reference level (see the note below):

$$
\ln r_{n+m} = 2.
$$

Then

$$
2 = 2^m \ln r_n.
$$

Hence

$$
2^m = \frac{2}{\ln r_n}.
$$

Taking $\log_2$,

$$
m = \log_2 2 - \log_2\left(\ln r_n\right),
$$

so

$$
m = 1 - \log_2\left(\ln r_n\right).
$$

Adding this fractional offset to the current iteration $n$ gives

$$
\nu = n + m,
$$

and therefore

$$
\boxed{\nu = n + 1 - \log_2\left(\ln r_n\right)}
$$

or, since $r_n = \lvert z_n \rvert$,

$$
\boxed{\nu = n + 1 - \frac{\ln\left(\ln \lvert z_n \rvert\right)}{\ln 2}}.
$$

Thus the "+1" does **not** represent another actual Mandelbrot iteration. It is part of the chosen normalization of the continuous iteration coordinate.

Changing this additive constant merely translates the palette along its color scale; it does not destroy the smoothing.

**Note:** The choice $\ln r_{n+m}=2$ is a normalization convention selected to recover the commonly used $+1$ form; it should not be confused with the escape criterion $\lvert z \rvert = 2$.

## 9. Why a Logarithm of a Logarithm Appears

The two logarithms can now be understood intuitively.

After escape,

$$
\lvert z_{n+1} \rvert \approx \lvert z_n \rvert^2.
$$

Thus the magnitude undergoes repeated squaring:

$$
r,\ r^2,\ r^4,\ r^8,\ r^{16},\ldots
$$

The **first logarithm** converts powers into multiplication:

$$
\ln r,\ 2\ln r,\ 4\ln r,\ 8\ln r,\ldots
$$

The quantity still doubles with each iteration.

The **second logarithm** converts this doubling into addition:

$$
\ln(\ln r),\ \ln(\ln r)\ +\ \ln 2,\ \ln(\ln r)\ +\ 2\ln 2,\ldots
$$

Dividing by $\ln 2$ makes each Mandelbrot iteration correspond to an increment of approximately one.

Thus,

$$
\frac{\ln\left(\ln \lvert z \rvert\right)}{\ln 2}
$$

acts like a continuous measure of how many iterations of exponential escape have occurred.

That is the mathematical reason for the double logarithm.

## 10. Translation into Practical C Language Code

For

$$
z_n = x + iy,
$$

the magnitude is

$$
\lvert z_n \rvert = \sqrt{x^2+y^2}.
$$

The formula could therefore be implemented directly as

```c
nu = n + 1.0 - log (log (sqrt (x * x + y * y))) / log (2.0);
```

However, the square root can conveniently be avoided. Because

$$
\ln\sqrt{x^2+y^2} = \frac{1}{2}\ln\left(x^2+y^2\right),
$$

we may write

```c
log_zn = log (x * x + y * y) / 2.0;
nu = n + 1.0 - log (log_zn) / log (2.0);
```

This is mathematically equivalent.

The commonly used formula

$$
\nu = n + 1 - \frac{\ln\left(\ln \lvert z_n \rvert\right)}{\ln 2}
$$

is represented directly in the current `long double` implementation by

```c
long double log_zn, smooth;

log_zn = logl ((r1 * r1) + (i1 * i1)) / 2.0L;
smooth = (long double) iterations + 1.0L
       - logl (log_zn) / logl (2.0L);
```

or, more compactly,

```c
smooth = (long double) iterations + 1.0L
       - log2l (logl ((r1 * r1) + (i1 * i1)) / 2.0L);
```

The difference between various formulae seen in Mandelbrot programs is often an additive constant, so several variants can produce visually similar results while shifting the palette phase. It is nevertheless useful to distinguish the formulae mathematically.

## 11. A Numerical Example

Suppose a point escapes on iteration

$$
n = 50
$$

and at that iteration

$$
\lvert z_{50} \rvert = 3.
$$

Then

$$
\ln \lvert z_{50} \rvert = \ln 3 \approx 1.098612.
$$

Next,

$$
\ln\left(\ln \lvert z_{50} \rvert\right)
= \ln(1.098612)
\approx 0.09403.
$$

Since

$$
\ln 2 \approx 0.693147,
$$

we obtain

$$
\frac{\ln\left(\ln \lvert z_{50} \rvert\right)}{\ln 2}
\approx 0.13565.
$$

Therefore,

$$
\nu \approx 50+1-0.13565,
$$

or

$$
\boxed{\nu \approx 50.86432}.
$$

The pixel is consequently not merely classified as "iteration 50." Its position within that escape band has been estimated continuously.

A nearby pixel might have

$$
\nu = 50.92,
$$

another

$$
\nu = 51.03,
$$

and another

$$
\nu = 51.17.
$$

A continuous palette can therefore vary smoothly across what would otherwise have been an abrupt boundary between integer iteration counts 50 and 51.

## 12. Why This Is Not Ordinary Spatial Interpolation

It is important to distinguish smooth iteration coloring from interpolation between neighbouring pixels.

No gradient of the two-dimensional image is required. We do not need to calculate

$$
\nabla n(x,y)
$$

or determine the normal to an iso-iteration contour.

Instead, every pixel independently supplies additional information through the final complex value $z_n$.

The integer escape count tells us approximately which band the pixel belongs to. The magnitude $\lvert z_n \rvert$ tells us how far through the escape process it has progressed.

The smooth value is therefore derived from the underlying Mandelbrot dynamics rather than from the geometry of neighbouring pixels.

This has several important consequences:

- no convolution or matrix operation is required;
- neighbouring pixels do not have to be examined;
- the calculation is amenable to parallel processing;
- increasing image resolution does not alter the mathematical definition; and
- the smoothing reflects the actual behaviour of the iterated function.

## 13. Why the Approximation Works Even Though $c$ Was Discarded

We used

$$
z_{n+1} \approx z_n^2
$$

instead of the exact expression

$$
z_{n+1} = z_n^2 + c.
$$

This approximation becomes increasingly accurate as the orbit escapes.

When $\lvert z_n \rvert$ is large,

$$
\lvert z_n^2 \rvert = \lvert z_n \rvert^2,
$$

whereas $c$ remains fixed.

Thus the relative importance of $c$ decreases rapidly:

$$
\frac{\lvert c \rvert}{\lvert z_n \rvert^2} \to 0.
$$

Consequently, the asymptotic behaviour approaches

$$
z_{n+1} \approx z_n^2.
$$

The smooth iteration formula should therefore be regarded as an **asymptotic interpolation of the escape dynamics**, not as an exact identity for every finite value of $z_n$.

One can improve its behavior by continuing an escaping orbit for additional iterations before calculating the smooth value. The iteration counter must also be advanced for those additional iterations before evaluating $\nu$; otherwise the near-invariance derived above is lost. When the magnitude has grown larger, the approximation

$$
z^2+c \approx z^2
$$

is even better.

## 14. Generalization to Other Powers

The appearance of $\ln 2$ is not accidental. It comes specifically from the exponent in the quadratic Mandelbrot iteration

$$
z_{n+1} = z_n^2+c.
$$

Consider instead the generalized "Multibrot" iteration

$$
z_{n+1} = z_n^p+c,
$$

where $p\ge2$, normally an integer.

After escape,

$$
\lvert z_{n+1} \rvert \approx \lvert z_n \rvert^p.
$$

Proceeding exactly as before gives

$$
\ln \lvert z_{n+1} \rvert \approx p\ln \lvert z_n \rvert.
$$

Taking another logarithm,

$$
\ln\left(\ln \lvert z_{n+1} \rvert\right)
\approx
\ln p + \ln\left(\ln \lvert z_n \rvert\right).
$$

Thus the double logarithm advances by $\ln p$ per iteration.

The corresponding normalized iteration count becomes

$$
\boxed{
\nu = n + 1 - \frac{\ln\left(\ln \lvert z_n \rvert\right)}{\ln p}
},\ where\ p\ \geq\ 2,\ normally\ and\ integer.
$$

up to the chosen additive normalization.

For the Mandelbrot set, $p=2$, and therefore

$$
\boxed{
\nu = n + 1 - \frac{\ln\left(\ln \lvert z_n \rvert\right)}{\ln 2}
}.
$$

## 15. Connection with the Escape-Rate Potential

There is a deeper mathematical interpretation.

For points outside the Mandelbrot set, define the escape-rate quantity

$$
g(c) = \lim_{n\to\infty}\frac{\ln \lvert z_n \rvert}{2^n}.
$$

This quantity is closely related to the Green function of the Mandelbrot set; the precise multiplicative normalization depends on the indexing convention used for the critical orbit. With $z_0=0$, the definition above is a convenient normalization for the present derivation.

We divide by $2^n$ because after escape,

$$
\ln \lvert z_{n+1} \rvert \approx 2\ln \lvert z_n \rvert.
$$

Thus

```math
\frac{\ln \lvert z_{n+1} \rvert}{2^{n+1}}
\approx
\frac{2\ln \lvert z_n \rvert}{2\cdot 2^n}
=
\frac{\ln \lvert z_n \rvert}{2^n}.
```

The quantity tends toward an invariant escape rate.

Taking the base-2 logarithm,

$$
\log_2 g(c)
\approx
\log_2\left(\ln \lvert z_n \rvert\right)-n.
$$

Rearranging,

$$
n-\log_2\left(\ln \lvert z_n \rvert\right)
\approx
-\log_2 g(c).
$$

Thus the continuous iteration count is, apart from the chosen additive normalization,

$$
\nu \approx -\log_2 g(c) + \text{constant}.
$$

Smooth iteration coloring is therefore directly related to the negative logarithm of the escape-rate potential, and hence to the Green-function description of escape from the Mandelbrot set.

## 16. Summary of the Derivation

Begin with the Mandelbrot iteration:

$$
z_{n+1} = z_n^2+c.
$$

After escape,

$$
z_{n+1} \approx z_n^2.
$$

Taking magnitudes,

$$
\lvert z_{n+1} \rvert \approx \lvert z_n \rvert^2.
$$

After $m$ additional iterations,

$$
\lvert z_{n+m} \rvert \approx \lvert z_n \rvert^{2^m}.
$$

Taking logarithms twice,

$$
\ln\left(\ln \lvert z_{n+m} \rvert\right)
\approx
m\ln 2 + \ln\left(\ln \lvert z_n \rvert\right).
$$

Therefore,

$$
\frac{\ln\left(\ln \lvert z_n \rvert\right)}{\ln 2}
$$

increases by approximately one for each iteration.

Subtracting this quantity from the integer iteration counter produces an approximately iteration-independent continuous measure of escape:

$$
n - \frac{\ln\left(\ln \lvert z_n \rvert\right)}{\ln 2}.
$$

With the conventional normalization, this becomes

$$
\boxed{
\nu = n + 1 - \frac{\ln\left(\ln \lvert z_n \rvert\right)}{\ln 2}
},
$$

which is the standard smooth or normalized Mandelbrot iteration count.

The two logarithms have a simple origin:

$$
\text{repeated squaring}
\xrightarrow{\ln}
\text{repeated doubling}
\xrightarrow{\ln}
\text{repeated addition}.
$$

It is this transformation from explosive quadratic growth into an approximately linear iteration coordinate that allows the integer escape bands of the Mandelbrot set to be replaced by a smoothly varying scalar field.

Copyright 2026 P. David Buchan (pdbuchan@gmail.com)

Licensed under the **GNU Free Documentation License, version 1.3 or any later version**, with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
