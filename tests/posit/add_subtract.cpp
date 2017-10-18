// add_subtract.cpp: functional tests for addition and subtraction
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../bitset/bitset_helpers.hpp"
#include "../../posit/posit_regime_lookup.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"

using namespace std;

// normalize creates a normalized number with the hidden bit installed: 1.bbbbbbbbb
template<size_t nbits>
void normalize(const std::bitset<nbits>& fraction, std::bitset<nbits>& number) {
	if (nbits == 3) return;
	number.set(nbits - 1); // set hidden bit
	int lb = nbits; lb -= 2;
	for (int i = lb; i >= 0; i--) {
		number.set(i, fraction[i + 1]);
	}
}
/*   h is hidden bit
*   h.bbbb_bbbb_bbbb_b...      fraction
*   0.000h_bbbb_bbbb_bbbb_b... number
*  >-.----<                    shift of 4
*/
template<size_t nbits>
void denormalize(const std::bitset<nbits>& fraction, int shift, std::bitset<nbits>& number) {
	if (nbits == 3) return;
	number.reset();
	if (shift <= nbits - 1) {
		number.set(nbits - 1 - shift); // set hidden bit
		for (int i = nbits - 2 - shift; i >= 0; i--) {
			number.set(i, fraction[i + 1 + shift]);
		}
	}
}

/*
	Testing the reciprocal nature of positive and negative posits
 */

/*
POSIT<4,0>
   #           Binary         Decoded         k-value            sign                        regime        exponent        fraction                         value
   0:             0000            0000               0               1                             1            -            ----                             0
   1:             0001            0001              -2               1                          0.25            -            ----                          0.25
   2:             0010            0010              -1               1                           0.5            -            0---                           0.5
   3:             0011            0011              -1               1                           0.5            -            1---                          0.75
   4:             0100            0100               0               1                             1            -            0---                             1
   5:             0101            0101               0               1                             1            -            1---                           1.5
   6:             0110            0110               1               1                             2            -            ----                             2
   7:             0111            0111               2               1                             4            -            ----                             4
   8:             1000            1000              -3              -1                           inf            -            ----                           inf
   9:             1001            1111               2              -1                             4            -            ----                            -4
  10:             1010            1110               1              -1                             2            -            ----                            -2
  11:             1011            1101               0              -1                             1            -            1---                          -1.5
  12:             1100            1100               0              -1                             1            -            0---                            -1
  13:             1101            1011              -1              -1                           0.5            -            1---                         -0.75
  14:             1110            1010              -1              -1                           0.5            -            0---                          -0.5
  15:             1111            1001              -2              -1                          0.25            -            ----                         -0.25
*/
bool ValidateAdditionPosit_4_0() {
	const int NR_TEST_CASES = 16;
	float input_values[NR_TEST_CASES] = {
		-4.0f, -2.0f, -1.5f, -1.0f, -0.75f, -0.5f, -0.25f, 0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 4.0f, INFINITY
	};

	bool bValid = true;
	posit<4, 0> pa, pb, psum, pref;
	float fa, fb;
	for (int i = 0; i < NR_TEST_CASES; i++) {
		fa = input_values[i];
		pa = fa;
		for (int j = 0; j < NR_TEST_CASES; j++) {
			fb = input_values[j];
			pb = fb;
			psum = pa + pb;
			pref = fa + fb;
			if (fabs(psum.to_double() - pref.to_double()) > 0.0001) {
				ReportBinaryArithmeticError("Posit<4,0> addition failed: ", "+", pa, pb, pref, psum);
				bValid = false;
			}
		}
	}
	return bValid;
}

bool ValidateSubtractionPosit_4_0() {
	const int NR_TEST_CASES = 16;
	float input_values[NR_TEST_CASES] = {
		-4.0f, -2.0f, -1.5f, -1.0f, -0.75f, -0.5f, -0.25f, 0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 4.0f, INFINITY
	};

	bool bValid = true;
	posit<4, 0> pa, pb, pdif, pref;
	float fa, fb;
	for (int i = 0; i < NR_TEST_CASES; i++) {
		fa = input_values[i];
		pa = fa;
		for (int j = 0; j < NR_TEST_CASES; j++) {
			fb = input_values[j];
			pb = fb;
			pdif = pa - pb;
			pref = fa - fb;
			if (fabs(pdif.to_double() - pref.to_double()) > 0.0001) {
				ReportBinaryArithmeticError("Posit<4,0> subtraction failed: ", "-", pa, pb, pref, pdif);
				bValid = false;
			}
		}
	}
	return bValid;
}

bool ValidateNegationPosit_4_0() {
	const int NR_TEST_CASES = 16;
	float target_values[NR_TEST_CASES] = {
		-4.0, -2.0, -1.5, -1.0, -0.75, -0.5, -0.25, 0.0, 0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 4.0, INFINITY
	};

	float golden_value[NR_TEST_CASES] = {
		+4.0, +2.0, +1.5, +1.0, +0.75, +0.5, +0.25, 0.0, -0.25, -0.5, -0.75, -1.0, -1.5, -2.0, -4.0, INFINITY
	};

	bool bValid = true;
	posit<4, 0> pa, pb, pref;
	for (int i = 0; i < NR_TEST_CASES; i++) {
		pa = target_values[i];
		pb = -pa;
		pref = golden_value[i];
		if (pb != pref) {
			ReportUnaryArithmeticError("Posit<4,0> negation failed: ", "-", pa, pref, pb);
			bValid = false;
		}
	}

	return bValid;
}

bool ValidateNegAdditionPosit_4_0() {
	float target_values[15] = {
		-4.0f, -2.0f, -1.5f, -1.0f, -0.75f, -0.5f, -0.25f, 0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 4.0f
	};

	bool bValid = true;
	posit<4, 0> pa, pb, psum;
	for (int i = 0; i < 15; i++) {
		pa =  target_values[i];
		pb = -pa;
		psum = pa + pb;
		if (fabs(psum.to_double()) > 0.0001) {
			cerr << "Posit<4,0> negated add failed: " << pa << " + " << pb << " != 0 instead it yielded " << psum << " " << components_to_string(psum) << endl;
			bValid = false;
		}
	}

	pa = INFINITY;
	pb = -pa;
	psum = pa + pb;
	if (!psum.isInfinite()) {
		cerr << "Posit<4,0> negated add failed: " << pa << " + " << pb << " != inf instead it yielded " << psum << " " << components_to_string(psum) << endl;
		bValid = false;
	}
	return bValid;
}

/*
POSIT<5,1>
   #           Binary         Decoded         k-value            sign                        regime        exponent        fraction                         value
   0:            00000           00000               0               1                             1               -               -----                             0
   1:            00001           00001              -3               1                      0.015625               -               -----                      0.015625
   2:            00010           00010              -2               1                        0.0625               0               -----                        0.0625
   3:            00011           00011              -2               1                        0.0625               1               -----                         0.125
   4:            00100           00100              -1               1                          0.25               0               0----                          0.25
   5:            00101           00101              -1               1                          0.25               0               1----                         0.375
   6:            00110           00110              -1               1                          0.25               1               0----                           0.5
   7:            00111           00111              -1               1                          0.25               1               1----                          0.75
   8:            01000           01000               0               1                             1               0               0----                             1
   9:            01001           01001               0               1                             1               0               1----                           1.5
  10:            01010           01010               0               1                             1               1               0----                             2
  11:            01011           01011               0               1                             1               1               1----                             3
  12:            01100           01100               1               1                             4               0               -----                             4
  13:            01101           01101               1               1                             4               1               -----                             8
  14:            01110           01110               2               1                            16               -               -----                            16
  15:            01111           01111               3               1                            64               -               -----                            64
  16:            10000           10000              -4              -1                           inf               -               -----                           inf
  17:            10001           11111               3              -1                            64               -               -----                           -64
  18:            10010           11110               2              -1                            16               -               -----                           -16
  19:            10011           11101               1              -1                             4               1               -----                            -8
  20:            10100           11100               1              -1                             4               0               -----                            -4
  21:            10101           11011               0              -1                             1               1               1----                            -3
  22:            10110           11010               0              -1                             1               1               0----                            -2
  23:            10111           11001               0              -1                             1               0               1----                          -1.5
  24:            11000           11000               0              -1                             1               0               0----                            -1
  25:            11001           10111              -1              -1                          0.25               1               1----                         -0.75
  26:            11010           10110              -1              -1                          0.25               1               0----                          -0.5
  27:            11011           10101              -1              -1                          0.25               0               1----                        -0.375
  28:            11100           10100              -1              -1                          0.25               0               0----                         -0.25
  29:            11101           10011              -2              -1                        0.0625               1               -----                        -0.125
  30:            11110           10010              -2              -1                        0.0625               0               -----                       -0.0625
  31:            11111           10001              -3              -1                      0.015625               -               -----                     -0.015625
*/
bool ValidateAdditionPosit_5_1() {
	const int NR_TEST_CASES = 32;
	float input_values[NR_TEST_CASES] = {
		-64.0f, -16.0f,  -8.0f, -4.0f, -3.0f, -2.0f, -1.5f, -1.0f, -0.75f, -0.5f, -0.375f, -0.25f, -0.125f, -0.0625f, -0.015625f, 0.0f, 
		0.015625f, 0.0625f, 0.125f, 0.25f, 0.375f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 3.0f, 4.0f,  8.0f, 16.0f, 64.0f, INFINITY
	};

	bool bValid = true;
	posit<5, 1> pa, pb, psum, pref;
	float fa, fb;
	for (int i = 0; i < NR_TEST_CASES; i++) {
		fa = input_values[i];
		pa = fa;
		for (int j = 0; j < NR_TEST_CASES; j++) {
			fb = input_values[j];
			pb = fb;
			psum = pa + pb;
			pref = fa + fb;
			if (fabs(psum.to_double() - pref.to_double()) > 0.0001) {
				ReportBinaryArithmeticError("Posit<5,1> addition failed: ", "+", pa, pb, pref, psum);
				bValid = false;
			}
		}
	}

	return bValid;
}

void TestPositArithmeticOperators(bool bValid, string posit_cfg, string op)
{
	if (!bValid) {
		cout << posit_cfg << " " << op << " FAIL" << endl;
	}
	else {
		cout << posit_cfg << " " << op << " PASS" << endl;
	}
}

int main(int argc, char** argv)
{
	TestPositArithmeticOperators(ValidateAdditionPosit_4_0(), "posit<4,0>", "addition");
	TestPositArithmeticOperators(ValidateNegationPosit_4_0(), "posit<4,0>", "negation");
	TestPositArithmeticOperators(ValidateNegAdditionPosit_4_0(), "posit<4,0>", "neg addition");
	TestPositArithmeticOperators(ValidateSubtractionPosit_4_0(), "posit<4,0>", "subtraction");

	TestPositArithmeticOperators(ValidateAdditionPosit_5_1(), "posit<5,1>", "addition");

	return 0;
}
