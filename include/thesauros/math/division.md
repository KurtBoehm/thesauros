Let $w$ be the number of bits in an integer, i.e. $32$ for 32 bit and $64$ for 64 bit.
The divisor $d$ is a `UFixed<w, 0>`, i.e. `UInt<w>`.
As an auxiliary value, we compute the inverse of the divisor, i.e. $d^{-1}$, as a fixed-point number with only fractional digits, i.e. `UFixed<0, 2 * w>` (as the paper shows, this number of digits is always sufficient).
This effectively means that only the fractional part $i$ of $d^{-1}$ is stored, i.e. $\def\fr#1{\operatorname{frac}\mathopen{}\left(#1\right)\mathclose{}} i = \fr{d^{-1}}$.

$i$ is equivalent to $d^{-1}$ (within the precision of `UFixed<0, 2 * w>`) for all positive integers $d$ save $1$, where $i = 0$.
While this discrepancy is unproblematic for computing the remainder, this case needs to be handled separately when dividing.

Another point to consider is the computation of $i$, which is equivalent to computing $\left\lceil \frac{2^n}{d} \right\rceil$ with $n = 2 w$ and interpreting the result appropriately.
The problem is that $2^n$ cannot be represented using $n$ bits, which is what we have.
Therefore, we transform this formula:

$$
\left\lceil \frac{2^{w}}{d} \right\rceil
= \left\lfloor \frac{2^{w} + d - 1}{d} \right\rfloor
= \left\lfloor \frac{2^{w} - 1}{d} + \frac{d}{d} \right\rfloor
= \left\lfloor \frac{2^{w} - 1}{d} \right\rfloor + 1
$$

This last formula is used in the implementation.

To compute $\left\lfloor\frac{x}{d}\right\rfloor$ for `UInt<w>` $x$, it now suffices to compute $x \cdot i$, which is a `UFixed<w, 2 * w>`, and remove the fractional part.
For $d = 1$ and $i = 0$, the result from this computation is always $0$, which requires this case to be handled using a branch.

To compute $x \bmod d$ for `UInt<w>` $x$, we exploit the relation $\def\fr#1{\operatorname{frac}\mathopen{}\left(#1\right)\mathclose{}} f = \fr{\frac{x}{d}} = \frac{x \bmod d}{d}$ and thus compute $\def\fr#1{\operatorname{frac}\mathopen{}\left(#1\right)\mathclose{}} x \bmod d = \left\lfloor \fr{x \cdot i} \cdot d \right\rfloor$.
Here, $i = 0$ for $d = 1$ leads to the correct results, as $x \bmod 1 = 0$ for every integer $x$.
