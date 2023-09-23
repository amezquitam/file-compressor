#include <compressor.hpp>
#include <string>
#include <cassert>

int main(int argc, char const *argv[]) {

    bytes to_compress = u8"ajsdhfiash ihskif hsdjfhasjkdfhakjsdfhas fkjshadgfkjasdbfuagshefoihdsflikahs ljhasdilkfh sadfhasdhfiasdhfiphdhf asfkjhasd lfkuhsadofal";

    bytes compressed = compress(to_compress);

    bytes uncompressed = uncompress(compressed);

    assert(uncompressed == to_compress);

    return 0;
}
