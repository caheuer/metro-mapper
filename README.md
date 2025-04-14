# Metro Mapper

Easily create simple metro maps with Metro Mapper!

You can quickly create good-looking metro maps with this tool. It will automatically arrange stations and lines such that there are no overlaps (does not always work) and stations are equally distant from each other on the same stretch.

Metro Mapper is still in development. It is made using C++ and SDL. A web version was compiled using Emscripten.

[Check out the web version on itch.io](https://chrisausdemklo.itch.io/metro-mapper)


## Future features

- Line names and tags on map
- Map markings such as bodies of water or fare zones
- Additional station information, such as accessibility
- Label placement manual override
- Station placement manual override
- Better station label auto placement
- Better auto station placement


## Libraries used
- [nlohmann/json](https://github.com/nlohmann/json)
- [stb/stb_image_write](https://github.com/nothings/stb/blob/master/stb_image_write.h)
- [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/)
