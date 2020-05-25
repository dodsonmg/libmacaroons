// Copyright 2020 Michael Dodson

#ifndef MACAROON_HPP_
#define MACAROON_HPP_

#include <string>
#include <vector>

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
  Macaroon(const struct macaroon* M_raw);
  Macaroon();

  /* TODO: Create a copy contstructor */
  /* TODO: Create a destructor */

  int initialise(const std::string location, const std::string key, const std::string identifier);
  int initialise(const struct macaroon* M_raw);
  bool initialised();  
  std::string serialise();
  int deserialise(std::string M_serialised);
  int add_first_party_caveat(const std::string predicate);
  int add_third_party_caveat(const std::string location, const std::string key, const std::string identifier);
  Macaroon prepare_for_request(const Macaroon D);
  void print_macaroon();
  struct macaroon* get_macaroon_raw();
  std::string get_macaroon_error(); 

private:
  void print_macaroon_error();

  struct macaroon* M_;
  enum macaroon_returncode err_;
};


/******************************************************************************
*  MacaroonVerifier class definition
******************************************************************************/
class MacaroonVerifier
{
  public:
    MacaroonVerifier(const std::string key);
    MacaroonVerifier();

    int initialise(const std::string key);
    bool initialised();
    int satisfy_exact(const std::string predicate);
    int satisfy_general(const std::string predicate);
    bool verify(Macaroon M, std::vector<Macaroon> MS = {});
    std::string get_verifier_error();    

  private:
    void print_verifier_error();

    struct macaroon_verifier* V_;
    std::string key_;
    enum macaroon_returncode err_;
};

#endif  // MACAROON_HPP_
