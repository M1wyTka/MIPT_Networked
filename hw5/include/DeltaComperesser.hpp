#include <Packet.hpp>

class Compresser
{
public:
    Compresser();
    ~Compresser();


    void Compress(Packet* packet);
    void Decomress(Packet* packet);
private:

};