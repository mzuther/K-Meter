# K-Meter

*Implementation of a [K-System meter] according to Bob Katz&rsquo;
specifications.*

![Screenshot](./doc/include/images/kmeter.png)

## About

K-Meter supports mono, stereo and 5.1 surround sound signals.  All
meters have been thoroughly validated.  The average meter reads either
**RMS** levels or **ITU BS.1770-1** loudness weighted levels.

*K-Meter was featured in an article by [NPR Labs].  Also, some users
have reported that they use K-Meter for teaching, while others use it
professionally such as in post-production.*

## Download

K-Meter can be downloaded in the [releases][] section (you may need to
click on "Assets").

I have dropped LV2 plug-in support for good.  However, you can still
use LV2 - just download version **v2.8.1**.

## Documentation

For documentation, licenses and further information, please see the
[manual][] and the directory [doc][].

## FAQ

### K-Meter crashes when the plug-in is loaded / opened

1. K-Meter requires a processor which supports the SSE2 instruction
   set (if you run at Windows 8 and above, it does).  On Windows, you
   might also have to install the [Visual C++ Redistributable for
   Visual Studio 2017][VC++ Redist].

2. K-Meter comes with a folder called `kmeter`.  This folder must be
   located in the same folder as the plug-in, so please copy it along
   and try again!

### Where are the LV2 plug-ins?

- The last release with LV2 plug-ins was version **v2.8.1**.  You can
  still download them from the "Releases" page.

### The stand-alone version does not work

- Unfortunately, I cannot do anything about it as I did not code this
  part of the application.  The stand-alone works well for me - try
  using another sound card or deleting the settings file.

## Code of conduct

Please read the [code of conduct][COC] before asking for help, filing
bug reports or contributing to this project.  Thanks!

## License

Copyright (c) 2010-2022 Martin Zuther

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Thank you for using free software!

## VST2 plug-ins

Technically, my VST2 plug-ins are not free software.  However, I
chose to provide them for your convenience.  All other binaries really
are free software in the sense of the Free Software Foundation.

*VST is a trademark of Steinberg Media Technologies GmbH, registered
in Europe and other countries.*


[COC]:             https://github.com/mzuther/K-Meter/tree/master/CODE_OF_CONDUCT.markdown
[doc]:             https://github.com/mzuther/K-Meter/tree/master/doc/
[manual]:          https://github.com/mzuther/K-Meter/raw/master/doc/kmeter.pdf
[releases]:        https://github.com/mzuther/K-Meter/releases

[K-System meter]:  https://www.digido.com/portfolio-item/level-practices-part-2/
[NPR Labs]:        http://www.nprlabs.org/
[VC++ Redist]:     https://www.visualstudio.com/downloads/
