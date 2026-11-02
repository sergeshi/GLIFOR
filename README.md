# GLIFOR

GLIFOR is a C++17 / Qt 6 desktop font-editor prototype. This foundation focuses on evaluating the editing workspace before implementing a production font engine.

## Prototype features
- Glyph list with starter glyphs A, B and O.
- Central glyph canvas with 1000 UPM grid, baseline, side bearings and advance width.
- Click to add outline nodes; close or clear the polygon.
- Add a glyph by entering a character.
- Edit advance width.
- Open and save editable projects as JSON.

The current outline format is polygonal. OTF/TTF import/export, Bézier handles, hinting and kerning are not implemented yet.

## Build on Windows / PowerShell

Requirements: Visual Studio 2022 with the C++ workload, CMake 3.21+, and Qt 6.4+ (MSVC kit).

~~~powershell
git clone https://github.com/sergeshi/GLIFOR.git
cd GLIFOR
cmake -S . -B build -DCMAKE_PREFIX_PATH "C:/Qt/6.x.x/msvc2022_64"
cmake --build build --config Release
.\build\Release\GLIFOR.exe
~~~

Replace the Qt path with the actual location of your installed Qt MSVC kit.

## Next milestones
1. Bézier nodes and handles, contour selection, undo/redo.
2. Glyph metrics, side bearings and spacing view.
3. Unicode character map and glyph naming.
4. OTF/TTF import/export backend.
5. Kerning, components, masters and variable-font axes.
