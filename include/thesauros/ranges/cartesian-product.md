# Advancing a Cartesian Product Iterator

Let $f: \mathbb{N}_0^d \to \mathbb{N}_0$ be a mapping from $d$-dimensional positions in a space with dimensions $n_i$ in dimension $i$ to one-dimensional indices of the following form:

$$
f(p) = \sum_{i=1}^d p_i \cdot \prod_{j=i+1}^d n_j
$$

## Goal

Compute a given $d$-dimensional position $q$ based on a given position $p$ such that $f(q) = f(p) + \delta$ for some given offset $\delta \in \mathbb{N}$.

## Algorithm

We start out by considering dimension $i = d$. For the following discussion, it is important to note that for this $i$, the product in the corresponding term of $f$’s definition is $1$ and all other terms include $n_d$ as factor, meaning that $f(p) \bmod n_d = p_d$.
To transform $p_d$ into $q_d$, we use modular arithmetic:

$$
q_d
= f(q) \bmod n_d
= (f(p) + \delta) \bmod n_d
= ((f(p) \bmod n_d) + \delta) \bmod n_d
= (p_d + \delta) \bmod n_d
$$

The remaining components of $q$ are computed recursively, with $p' = (p_1, \dots, p_{d-1})$ as the given position and

$$
\left\lfloor \frac{p_d + \delta}{n_d} \right\rfloor
$$

as the given offset, i.e. the part of $p_d + \delta$ that is not represented in $q_d$ divided by $n_d$, which is no longer a factor in $f(p')$ due to

$$
f(p') = \frac{f(p) - p_d}{n_d} = \left\lfloor \frac{f(p)}{n_d} \right\rfloor.
$$

## Handling Negative Offsets

The formulae described in the previous section still work when $\delta < 0$, assuming that the remainder represented by $\bmod$ is non-negative.
For translating the formula into code, however, it is important to ensure that the remainder is non-negative and the quotient is actually rounded downwards, which is not what happens in C++ (and other programming languages), which round towards zero and determine non-positive remainders if the quotient is negative.
