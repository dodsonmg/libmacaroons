// Copyright 2020 Michael Dodson

#ifndef MACAROON_HPP_
#define MACAROON_HPP_

#include <string>

/* macaroons */
#include "macaroons.h"

/******************************************************************************
*  Macaroon class definition
******************************************************************************/
class Macaroon
{
public:
  Macaroon(const std::string location, const std::string key, const std::string identifier);
  Macaroon(const std::string M_serialised);
  Macaroon();

  /* TODO: Create a copy contstructor */
  /* TODO: Create a destructor */

  std::string serialise();
  int deserialise(std::string M_serialised);
  int add_first_party_caveat(const std::string predicate);
  int add_third_party_caveat(void);  // not implemented
  bool initialised();
  void print_macaroon();
  struct macaroon* get_macaroon_raw();

private:
  int create_macaroon(const std::string location, const std::string key, const std::string identifier);
  void print_macaroon_error(enum macaroon_returncode err);

  struct macaroon* M_;
};


/******************************************************************************
*  MacaroonVerifier class definition
******************************************************************************/
class MacaroonVerifier
{
  public:
    MacaroonVerifier(std::string key);

    int satisfy_exact(const std::string predicate);
    int satisfy_general(const std::string predicate);
    int verify(Macaroon M);  // eventually this needs to take in a tree of macaroons for 3rd party verifiers
    std::string get_verifier_error();    

  private:
    void print_verifier_error();

    struct macaroon_verifier* V_;
    std::string key_;
    enum macaroon_returncode err_;
};

#endif  // MACAROON_HPP_
