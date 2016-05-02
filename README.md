# lua-bgfx
BGFX bindings for Lua.

[API Status](https://docs.google.com/spreadsheets/d/1GLlaigjpsc9rYyA8neXsRHI6jQZ_JfiN4Of8BJnct2s/edit?usp=sharing)

# Compiling
## Windows
First, you'll need to grab lua-bgfx and all the submodules: `git clone https://github.com/excessive/lua-bgfx.git --recursive`

Now, since SDL2 isn't likely to be available system-wide, you'll need that: `git clone https://github.com/spurious/SDL-mirror.git extern/sdl`

That should be all, either run `genie vs2013` in the `scripts` folder or let `build.bat` take care of everything for you.

**Note**: *The build script assumes vs2013 - 12/15 might work, but you'll need to edit the `%GENIE% vs2013` line.* 

For the exceedingly lazy, you can run the test program with `run.bat` 

## Linux
First, install `SDL2-devel` and `luajit-devel` (on Fedora, you can just `dnf install SDL2-devel luajit-devel`

Now grab lua-bgfx and the submodules: `git clone https://github.com/excessive/lua-bgfx.git --recursive`

Then generate the project files: `./extern/bx/tools/bin/linux/genie gmake` 

Then make: `make -j9`


# License
**Note**: *The following is also in LICENSE.md.*

## The MIT License (MIT)
(c) 2016 Colby Klein <shakesoda@gmail.com>

(c) 2016 Landon Manning <lmanning17@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
