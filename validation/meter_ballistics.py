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
meterMinimumDecibel = lowestLevel - 20.0

print()
print('Lowest measurable level:   {:+7.2f} dB'.format(lowestLevel))
print('Meter minimum decibel:     {:+7.2f} dB'.format(meterMinimumDecibel))

result = meterMinimumDecibel * 0.99

print()
print('Position 00:12.600')
print('==================')
print('Fall time average (K-20):  {:+7.2f} dB'.format(20 + result))
print('Fall time average (K-14):  {:+7.2f} dB'.format(14 + result))
print('Fall time average (K-12):  {:+7.2f} dB'.format(12 + result))
print('Fall time average (Norm):  {:+7.2f} dB'.format(result))

result = meterMinimumDecibel * 0.01

print()
print('Position 00:25.200')
print('==================')
print('Rise time average (K-20):  {:+7.2f} dB'.format(20 + result))
print('Rise time average (K-14):  {:+7.2f} dB'.format(14 + result))
print('Rise time average (K-12):  {:+7.2f} dB'.format(12 + result))
print('Rise time average (Norm):  {:+7.2f} dB'.format(result))

result = -26.0

print()
print('Position 00:40.200')
print('==================')
print('Fall time peaking (K-20):  {:+7.2f} dB'.format(20 + result))
print('Fall time peaking (K-14):  {:+7.2f} dB'.format(14 + result))
print('Fall time peaking (K-12):  {:+7.2f} dB'.format(12 + result))
print('Fall time peaking (Norm):  {:+7.2f} dB'.format(result))

result = 0.0

print()
print('Position 00:40.200')
print('==================')
print('Rise time peaking (K-20):  {:+7.2f} dB'.format(20 + result))
print('Rise time peaking (K-14):  {:+7.2f} dB'.format(14 + result))
print('Rise time peaking (K-12):  {:+7.2f} dB'.format(12 + result))
print('Rise time peaking (Norm):  {:+7.2f} dB'.format(result))

print()
