#include <iostream>
#include <vector>

/* macaroons */
#include "macaroons/macaroons.hpp"

int
main(void)
{
    // Create a macaroon and serialise it
    Macaroon M;
    MacaroonVerifier V;
    std::string location = "https://www.unused.com/";
    std::string key = "a_bad_key";
    std::string identifier = "bad_key_id";
    std::string fpc_0 = "access = r/w/x";
    std::string fpc_1 = "access = r";

    M.initialise(location, key, identifier);
    V.initialise(key);

    std::cout << std::endl;

    // test first party caveats
    std::cout << "Testing first party caveats" << std::endl;
    std::cout << "- Adding first party caveat to M" << std::endl;
    M.add_first_party_caveat(fpc_0);
    M.print_macaroon();
    if(V.verify(M))  // expect this verification to fail (caveat not added to verifier)
    {
        std::cout << "- Verification: FAIL (shouldn't have passed)" << std::endl;
        return 0;
    }
    std::cout << "- Adding first party caveat to V" << std::endl;
    V.satisfy_exact(fpc_0);
    if(!V.verify(M))  // expect this verification to pass (caveat added to verifier)
    {
        std::cout << "- Verification: FAIL (should have passed)" << std::endl;
        return 0;
    }
    std::cout << "First party caveats:  PASS\n" << std::endl;

    std::cout << std::endl;

    // test third party caveats
    std::cout << "Testing third party caveats" << std::endl;
    std::cout << "- Adding third party caveat to M" << std::endl;

    // create third party caveat
    std::string third_party_location = "https://www.unused_third_party.com/";
    std::string third_party_key = "another_bad_key";
    std::string third_party_predicate = "name = billyjoe";  // used by third party, not by the local verifier
    // send_key_to_third_party(third_party_key, third_party_predicate)
    // third_party_identifier = get_id_from_third_party()
    std::string third_party_identifier = "another_bad_key_id";

    M.add_third_party_caveat(third_party_location, third_party_key, third_party_identifier);
    M.print_macaroon();
    if(V.verify(M))  // expect this verification to fail (no discharge macaroon provided)
    {
        std::cout << "- Verification: FAIL (should not have passed)" << std::endl;
        return 0;
    }

    // create discharge caveat (this should be done by the third party)
    std::cout << "- Creating discharge caveat D" << std::endl;
    Macaroon D;
    D.initialise(third_party_location, third_party_key, third_party_identifier);
    D.print_macaroon();

    std::vector<Macaroon> MS = {D};
    if(V.verify(M, MS))  // expect this verification to fail (discharge macaroon not bound to M)
    {
        std::cout << "- Verification: FAIL (should have failed)" << std::endl;
        return 0;
    }

    // Bind discharge macaroon to original macaroon
    std::cout << "- Binding discharge caveat D to M" << std::endl;
    Macaroon DP = M.prepare_for_request(D);
    DP.print_macaroon();

    MS = {DP};
    if(!V.verify(M, MS))  // expect this verification to pass (discharge macaroon provided)
    {
        std::cout << "- Verification: FAIL (should have passed)" << std::endl;
        return 0;
    }

    std::cout << "Third party caveats:  PASS\n" << std::endl;
}