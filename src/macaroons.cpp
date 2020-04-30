// Copyright 2020 Michael Dodson

#include <string>
#include <iostream>

#include "macaroons/macaroons.hpp"

/* macaroons */
#include "macaroons/macaroons.h"

/******************************************************************************
*  Macaroon class implementation
******************************************************************************/
Macaroon::Macaroon(const std::string location, const std::string key, const std::string identifier)
{
    int result = initialise(location, key, identifier);

    if (result != 0) { abort(); }
}

Macaroon::Macaroon(const std::string M_serialised)
{
    int result = deserialise(M_serialised);

    if (result != 0) { abort(); }
}

Macaroon::Macaroon()
{
    M_ = NULL;
}

std::string
Macaroon::serialise()
{
    size_t buf_sz = 0;
    unsigned char* buf = NULL;

    /* DEBUG */
    // print_macaroon(M_);
    /* END DEBUG */

    // If M is NULL
    if(!M_)
    {
        return "";
    }

    buf_sz = macaroon_serialize_size_hint(M_, MACAROON_V1);
    buf = (unsigned char*)malloc(buf_sz);

    macaroon_serialize(M_, MACAROON_V1, buf, buf_sz, &err_);

    std::string serialised( (const char *)buf );

    return serialised;
}

int
Macaroon::add_first_party_caveat(const std::string predicate)
{
    const unsigned char* ppredicate = (const unsigned char*)predicate.c_str();
    size_t ppredicate_sz = predicate.size();

    M_ = macaroon_add_first_party_caveat(M_, ppredicate, ppredicate_sz, &err_);

    if(!M_)
    {
        return -1;
    }

    return 0;
}

int
Macaroon::add_third_party_caveat(void)
{
    return -1;
}

int
Macaroon::deserialise(std::string M_serialised)
{
    const unsigned char* data = (const unsigned char*)M_serialised.c_str();
    size_t data_sz = M_serialised.size();

    M_ = macaroon_deserialize(data, data_sz, &err_);

    if(!M_)
    {
        return -1;
    }

    /* DEBUG */
    // print_macaroon(M_);
    /* END DEBUG */

    return 0;
}

int
Macaroon::initialise(const std::string location, const std::string key, const std::string identifier)
{
    const unsigned char* plocation = (const unsigned char*)location.c_str();
    const unsigned char* pkey = (const unsigned char*)key.c_str();
    const unsigned char* pidentifier = (const unsigned char*)identifier.c_str();

    size_t location_sz = location.size();
    size_t key_sz = key.size();
    size_t identifier_sz = identifier.size();

    M_ = macaroon_create(plocation, location_sz, 
                        pkey, key_sz, 
                        pidentifier, identifier_sz, &err_);

    /* DEBUG */
    // print_macaroon(M);
    /* END DEBUG */

    // if macaroon creation fails, M will be NULL and an error code (hopefully) returned
    if(!M_)
    {
        return -1;
    }

    return 0;
}

bool
Macaroon::initialised()
{
    if(M_)
    {
        return true;
    }

    return false;
}

struct macaroon*
Macaroon::get_macaroon_raw()
{
    return M_;
}

void
Macaroon::print_macaroon_error()
{
  std::cout << "Error (" << err_ << "): ";
  switch(err_) {
      case MACAROON_SUCCESS : std::cout << "MACAROON_SUCCESS" << std::endl; break;
      case MACAROON_OUT_OF_MEMORY : std::cout << "MACAROON_OUT_OF_MEMORY" << std::endl; break;
      case MACAROON_HASH_FAILED : std::cout << "MACAROON_HASH_FAILED" << std::endl; break;
      case MACAROON_INVALID : std::cout << "MACAROON_INVALID" << std::endl; break;
      case MACAROON_TOO_MANY_CAVEATS : std::cout << "MACAROON_TOO_MANY_CAVEATS" << std::endl; break;
      case MACAROON_CYCLE : std::cout << "MACAROON_CYCLE" << std::endl; break;
      case MACAROON_BUF_TOO_SMALL : std::cout << "MACAROON_BUF_TOO_SMALL" << std::endl; break;
      case MACAROON_NOT_AUTHORIZED : std::cout << "MACAROON_NOT_AUTHORIZED" << std::endl; break;
      case MACAROON_NO_JSON_SUPPORT : std::cout << "MACAROON_NO_JSON_SUPPORT" << std::endl; break;
      case MACAROON_UNSUPPORTED_FORMAT : std::cout << "MACAROON_UNSUPPORTED_FORMAT" << std::endl; break;
  }
}

std::string
Macaroon::get_macaroon_error()
{
    std::string err;
    switch(err_) {
        case MACAROON_SUCCESS : err = "MACAROON_SUCCESS"; break;
        case MACAROON_OUT_OF_MEMORY : err = "MACAROON_OUT_OF_MEMORY"; break;
        case MACAROON_HASH_FAILED : err = "MACAROON_HASH_FAILED"; break;
        case MACAROON_INVALID : err = "MACAROON_INVALID"; break;
        case MACAROON_TOO_MANY_CAVEATS : err = "MACAROON_TOO_MANY_CAVEATS"; break;
        case MACAROON_CYCLE : err = "MACAROON_CYCLE"; break;
        case MACAROON_BUF_TOO_SMALL : err = "MACAROON_BUF_TOO_SMALL"; break;
        case MACAROON_NOT_AUTHORIZED : err = "MACAROON_NOT_AUTHORIZED"; break;
        case MACAROON_NO_JSON_SUPPORT : err = "MACAROON_NO_JSON_SUPPORT"; break;
        case MACAROON_UNSUPPORTED_FORMAT : err = "MACAROON_UNSUPPORTED_FORMAT"; break;
    }

    return err;
}

void
Macaroon::print_macaroon()
{
    size_t data_sz = 0;
    char* data = NULL;
    data_sz = macaroon_inspect_size_hint(M_);
    data = (char*)malloc(data_sz);

    macaroon_inspect(M_, data, data_sz, &err_);
    std::cout << data << std::endl;
}


/******************************************************************************
*  MacaroonVerifier class implementation
******************************************************************************/
MacaroonVerifier::MacaroonVerifier(const std::string key)
{
    int result = initialise(key);

    if(result != 0) { abort(); }
}

MacaroonVerifier::MacaroonVerifier()
{
    V_ = NULL;
}

int
MacaroonVerifier::initialise(const std::string key)
{
    V_ = macaroon_verifier_create();
    key_ = key;

    if(V_)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

bool
MacaroonVerifier::initialised()
{
    if(V_)
    {
        return true;
    }

    return false;
}

int
MacaroonVerifier::satisfy_exact(const std::string predicate)
{
    /*
    adds a caveat to the verifier that must be satisfied exactly

    returns 0 if added successfully, -1 otherwise
    */

    int result;
    const unsigned char* ppredicate = (const unsigned char*)predicate.c_str();
    size_t ppredicate_sz = predicate.size();

    result = macaroon_verifier_satisfy_exact(V_, ppredicate, ppredicate_sz, &err_);

    return result;
}

int
MacaroonVerifier::satisfy_general(const std::string predicate)
{
    /*
    adds a caveat to the verifier that is satisfied generally

    returns 0 if added successfully, -1 otherwise

    not implemented yet, so just returns -1 (i.e., failure)
    */
   std::cout << "MacaroonVerifier::satisfy_general() is not implemented yet." << std::endl;
   std::cout << "Attempting to set predicate " << predicate << std::endl;
   
   return -1;
}

bool
MacaroonVerifier::verify(Macaroon M){
    /*
    verifies a macaroon M against the private verifier V_

    TODO:  eventually this needs to take in a tree of macaroons for 3rd party verifiers 
    */    

    int result;
    const unsigned char* pkey = (const unsigned char*)key_.c_str();
    size_t pkey_sz = key_.size();

    // eventually these should be implemented
    struct macaroon** MS = NULL;
    size_t MS_sz = 0;

    result = macaroon_verify(V_, M.get_macaroon_raw(), pkey, pkey_sz, MS, MS_sz, &err_);

    if (result == 0)
    {
        return true;
    }

    return false;
}

void
MacaroonVerifier::print_verifier_error()
{
  std::cout << "Error (" << err_ << "): ";
  switch(err_) {
      case MACAROON_SUCCESS : std::cout << "MACAROON_SUCCESS" << std::endl; break;
      case MACAROON_OUT_OF_MEMORY : std::cout << "MACAROON_OUT_OF_MEMORY" << std::endl; break;
      case MACAROON_HASH_FAILED : std::cout << "MACAROON_HASH_FAILED" << std::endl; break;
      case MACAROON_INVALID : std::cout << "MACAROON_INVALID" << std::endl; break;
      case MACAROON_TOO_MANY_CAVEATS : std::cout << "MACAROON_TOO_MANY_CAVEATS" << std::endl; break;
      case MACAROON_CYCLE : std::cout << "MACAROON_CYCLE" << std::endl; break;
      case MACAROON_BUF_TOO_SMALL : std::cout << "MACAROON_BUF_TOO_SMALL" << std::endl; break;
      case MACAROON_NOT_AUTHORIZED : std::cout << "MACAROON_NOT_AUTHORIZED" << std::endl; break;
      case MACAROON_NO_JSON_SUPPORT : std::cout << "MACAROON_NO_JSON_SUPPORT" << std::endl; break;
      case MACAROON_UNSUPPORTED_FORMAT : std::cout << "MACAROON_UNSUPPORTED_FORMAT" << std::endl; break;
  }
}

std::string
MacaroonVerifier::get_verifier_error()
{
    std::string err;
    switch(err_) {
        case MACAROON_SUCCESS : err = "MACAROON_SUCCESS"; break;
        case MACAROON_OUT_OF_MEMORY : err = "MACAROON_OUT_OF_MEMORY"; break;
        case MACAROON_HASH_FAILED : err = "MACAROON_HASH_FAILED"; break;
        case MACAROON_INVALID : err = "MACAROON_INVALID"; break;
        case MACAROON_TOO_MANY_CAVEATS : err = "MACAROON_TOO_MANY_CAVEATS"; break;
        case MACAROON_CYCLE : err = "MACAROON_CYCLE"; break;
        case MACAROON_BUF_TOO_SMALL : err = "MACAROON_BUF_TOO_SMALL"; break;
        case MACAROON_NOT_AUTHORIZED : err = "MACAROON_NOT_AUTHORIZED"; break;
        case MACAROON_NO_JSON_SUPPORT : err = "MACAROON_NO_JSON_SUPPORT"; break;
        case MACAROON_UNSUPPORTED_FORMAT : err = "MACAROON_UNSUPPORTED_FORMAT"; break;
    }

    return err;
}