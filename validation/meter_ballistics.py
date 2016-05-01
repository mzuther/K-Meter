#! /usr/bin/env python3

# ----------------------------------------------------------------------------
#
#  K-Meter
#  =======
#  Implementation of a K-System meter according to Bob Katz' specifications
#
#  Copyright (c) 2010-2016 Martin Zuther (http://www.mzuther.de/)
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#  Thank you for using free software!
#
# ----------------------------------------------------------------------------


# see "MeterBallistics::fMeterMinimumDecibel"
lowestLevel = -90.01
peakToAverageCorrection = 3.0103
meterMinimumDecibel = lowestLevel - peakToAverageCorrection

print()
print('Lowest measurable level:   {:+6.2f} dB'.format(lowestLevel))
print('Peak to avg correction:    {:+6.2f} dB'.format(peakToAverageCorrection))
print('Meter minimum decibel:     {:+6.2f} dB'.format(meterMinimumDecibel))

result = meterMinimumDecibel * 0.99

print()
print('Fall time average (K-20):  {:+6.2f} dB'.format(20 + result))
print('Fall time average (K-14):  {:+6.2f} dB'.format(14 + result))
print('Fall time average (K-12):  {:+6.2f} dB'.format(12 + result))
print('Fall time average (Norm):  {:+6.2f} dB'.format(result))

result = meterMinimumDecibel * 0.01

print()
print('Rise time average (K-20):  {:+6.2f} dB'.format(20 + result))
print('Rise time average (K-14):  {:+6.2f} dB'.format(14 + result))
print('Rise time average (K-12):  {:+6.2f} dB'.format(12 + result))
print('Rise time average (Norm):  {:+6.2f} dB'.format(result))

result = -26.0

print()
print('Fall time peak (K-20):  {:+6.2f} dB'.format(20 + result))
print('Fall time peak (K-14):  {:+6.2f} dB'.format(14 + result))
print('Fall time peak (K-12):  {:+6.2f} dB'.format(12 + result))
print('Fall time peak (Norm):  {:+6.2f} dB'.format(result))

result = 0.0

print()
print('Rise time peak (K-20):  {:+6.2f} dB'.format(20 + result))
print('Rise time peak (K-14):  {:+6.2f} dB'.format(14 + result))
print('Rise time peak (K-12):  {:+6.2f} dB'.format(12 + result))
print('Rise time peak (Norm):  {:+6.2f} dB'.format(result))

print()
