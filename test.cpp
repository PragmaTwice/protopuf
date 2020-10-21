
#include <iostream>
#include <zigzag.h>
#include <varint.h>

using namespace std;
using namespace pp;

int main() {
    sint_zigzag<4> a(-1);
    array b{static_cast<byte>(0b10), static_cast<byte>(0b10000000)};
    array c = int_to_bytes<2>(0x8002);
    cout << hex;
    cout << -1 << " " << a.get_underlying() << " " << a.get() << "\n";
    cout << bytes_to_int(span(b)) << " " << bytes_to_int(span(c)) << "\n";
    array<byte, 10> d{};
    varint_coder<int>::encode((unsigned)-1, d);
    cout << hex << (int)d[0] << " " << (int)d[1]  << " " << (int)d[2] << " " << (int)d[3] << " " << (int)d[4] << "\n";
    cout << dec << varint_coder<int>::decode(span(d)).first << "\n";
    varint_coder<sint_zigzag<2>>::encode(sint_zigzag<2>(300), d);
    cout << hex << (int)d[0] << " " << (int)d[1] << "\n";
    cout << dec << varint_coder<sint_zigzag<2>>::decode(span(d)).first.get() << "\n";
    varint_coder<int>::encode(unsigned(300), d);
    cout << hex << (int)d[0] << " " << (int)d[1] << "\n";
    cout << dec << varint_coder<int>::decode(span(d)).first << "\n";
    cout << coder < integer_coder<unsigned>> << "\n";
}
