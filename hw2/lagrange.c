//
// Created by umit on 18.04.2021.
//

#include "lagrange.h"


double Li(int i, int n, const double x[], double X) {
  int j;
  double prod = 1;
  for (j = 0; j <= n; j++) {
    if (j != i)
      prod = prod * (X - x[j * 2]) / (x[i * 2] - x[j * 2]);
  }
  return prod;
} /*Function to evaluate Pn(x) where Pn is the Lagrange interpolating polynomial
   * of degree n*/

double Pn(int n, double x[], double X) {
  double sum = 0;
  int i;
  for (i = 0; i <= n; i++) {
    sum = sum + Li(i, n, x, X) * x[2 * i + 1];
  }
  return sum;
}