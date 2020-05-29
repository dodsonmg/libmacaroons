#include "macaroons/macaroons.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

using namespace macaroons;

std::string hex_encode(const std::vector<unsigned char> &data)
{
    std::stringstream stream;
    stream.fill('0');

    for (auto c : data)
        stream << std::hex << std::setw(2) << static_cast<unsigned int>(c);

    return stream.str();
}

void using_third_party_caveats(Verifier &V);

int main()
{

    auto secret = "this is our super secret key; only we should know it";
    auto pub = "we used our secret key";
    auto location = "http://mybank/";

    Macaroon M{location, secret, pub};

    assert(M.identifier() == pub);
    assert(M.location() == location);
    assert(hex_encode(M.signature()) ==
        "e3d9e02908526c4c0039ae15114115d97fdd68bf2ba379b342aaf0f617d0552f");
    assert(M.serialize() == "MDAxY2xvY2F0aW9uIGh0dHA6Ly9teWJhbmsvCjAwMjZpZGVudG"
                            "lmaWVyIHdlIHVzZWQgb3VyIHNlY3JldCBrZXkKMDAyZnNpZ25h"
                            "dHVyZSDj2eApCFJsTAA5rhURQRXZf91ovyujebNCqvD2F9BVLw"
                            "o");

    std::cout << M.inspect() << std::endl;

    M = M.add_first_party_caveat("account = 3735928559");

    std::cout << std::endl;
    std::cout << M.inspect() << std::endl;

    M = M.add_first_party_caveat("time < 2030-01-01T00:00");
    assert(hex_encode(M.signature()) ==
        "d60992fd99c8e8ee9d05c61d71268b2e6a76fa4890ff4af6abd8bbbb4dfde5cb");

    M = M.add_first_party_caveat("email = alice@example.org");
    assert(hex_encode(M.signature()) ==
        "70754d73ccf8e4740d639ef8a26d337db82205ea4b37e683002849577afa6f83");

    std::cout << std::endl;
    std::cout << M.inspect() << std::endl;

    auto msg = M.serialize();
    assert(M == Macaroon::deserialize(msg));

    M = Macaroon::deserialize(msg);

    std::cout << std::endl;
    std::cout << M.inspect() << std::endl;

    Verifier V;

    try {
        V.verify(M, secret);
    }
    catch (exception::NotAuthorized &e) {
        std::cout << e.what() << std::endl;
    }

    assert(!V.verify_unsafe(M, secret));

    V.satisfy_exact("account = 3735928559");
    V.satisfy_exact("email = alice@example.org");
    V.satisfy_exact("IP = 127.0.0.1");
    V.satisfy_exact("browser = Chrome");
    V.satisfy_exact("action = deposit");

    auto check_time = [](const std::string &caveat) {
        if (caveat.find("time < ") != 0)
            return false;

        std::tm tm;
        std::istringstream ss(caveat.substr(7));
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M");
        if (ss.fail())
            return false;

        auto t = std::mktime(&tm);
        return std::time(nullptr) < t;
    };

    assert(check_time("time < 2030-01-01T00:00"));
    assert(!check_time("time < 2014-01-01T00:00"));
    assert(!check_time("account = 3735928559"));

    V.satisfy_general(std::move(check_time));
    assert(V.verify_unsafe(M, secret));

    auto N = M.add_first_party_caveat("action = deposit");
    assert(V.verify_unsafe(N, secret));

    // Unknown caveat
    N = M.add_first_party_caveat("OS = Windows XP");
    assert(!V.verify_unsafe(N, secret));

    // False caveat
    N = M.add_first_party_caveat("time < 2014-01-01T00:00");
    assert(!V.verify_unsafe(N, secret));

    // Bad secret
    assert(!V.verify_unsafe(M, "this is not the secret we were looking for"));

    // Incompetent hackers trying to change the signature
    N = Macaroon::deserialize(
        "MDAxY2xvY2F0aW9uIGh0dHA6Ly9teWJhbmsvCjAwMjZpZGVudGlmaWVyIHdlIHVzZWQgb3"
        "VyIHNl\nY3JldCBrZXkKMDAxZGNpZCBhY2NvdW50ID0gMzczNTkyODU1OQowMDIwY2lkIH"
        "RpbWUgPCAyMDIw\nLTAxLTAxVDAwOjAwCjAwMjJjaWQgZW1haWwgPSBhbGljZUBleGFtcG"
        "xlLm9yZwowMDJmc2lnbmF0\ndXJlID8f19FL+bkC9p/"
        "aoMmIecC7GxdOcLVyUnrv6lJMM7NSCg==\n");

    std::cout << std::endl;
    std::cout << N.inspect() << std::endl;

    assert(M.signature() != N.signature());
    assert(!V.verify_unsafe(N, secret));

    using_third_party_caveats(V);
}

void using_third_party_caveats(Verifier &V)
{
    auto secret =
        "this is a different super-secret key; never use the same secret twice";
    auto pub = "we used our other secret key";
    auto location = "http://mybank/";

    Macaroon M{location, secret, pub};
    M = M.add_first_party_caveat("account = 3735928559");

    std::cout << std::endl;
    std::cout << M.inspect() << std::endl;


    // you'll likely want to use a higher entropy source to generate this key
    auto caveatKey = "4; guaranteed random by a fair toss of the dice";
    auto predicate = "user = Alice";

    // sendToAuth(caveat_key, predicate);
    // auto identifier = recvFromAuth();

    auto identifier = "this was how we remind auth of key/pred";
    M = M.add_third_party_caveat("http://auth.mybank/", caveatKey, identifier);

    std::cout << std::endl;
    std::cout << M.inspect() << std::endl;

    assert(M.third_party_caveats().size() == 1);
    assert(M.third_party_caveats().front().location() == "http://auth.mybank/");
    assert(M.third_party_caveats().front().identifier() ==
        "this was how we remind auth of key/pred");

    Macaroon D{"http://auth.mybank/", caveatKey, identifier};
    D = D.add_first_party_caveat("time < 2030-01-01T00:00");

    std::cout << std::endl;
    std::cout << D.inspect() << std::endl;

    auto DP = M.prepare_for_request(D);

    assert(hex_encode(D.signature()) ==
        "aa0371f6baec22abb9c0bda1050756cc58392dc88f4696962f5731e497359cb4");

    assert(V.verify_unsafe(M, secret, {DP}));
    assert(!V.verify_unsafe(M, secret, {D}));
}