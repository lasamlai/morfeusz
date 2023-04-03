/* Prolog interface to the morphological analyzer Morfeusz 2 (“Morfeusz Reloaded”) 
   Version for SWI Prolog release 5.6 and up (Unicode relases)

   Author: Marcin Woliński 
   This file is in the public domain. 

   To compile use this incantation on Linux:
   
   plld -v -shared -o morfeusz-swi.so morfeusz2-glueswi.cc -lmorfeusz2

   On Ubuntu 14.04 and later the line becomes:
   
   swipl-ld -v -shared -o morfeusz-swi.so morfeusz2-glueswi.cc -lmorfeusz2

   Here is how I compile the library for Windows using Linux version of MinGW32:
   i586-mingw32msvc-gcc -D_REENTRANT -D__SWI_PROLOG__ -I./ -I/usr/lib/swi-prolog/include -o morfeusz-swi.dll -shared morfeusz2-glueswi.cc  -L./ -lpl -lmorfeusz2

   SWI Prolog 6.x names its library libswipl.dll:
   i586-mingw32msvc-gcc -D_REENTRANT -D__SWI_PROLOG__ -I./ -I/usr/lib/swi-prolog/include -o morfeusz-swi.dll -shared morfeusz2-glueswi.cc  -L./ -lswipl -lmorfeusz2

   SWI Prolog 6.x, 7.x, 8.x on Win64 with 64-bit MinGW:
   To compile:
   • uncompress SWI Prolog Win64 installer in an empty directory (7z x swipl….exe),
   • copy morfeusz2.h, morfeusz2.dll, and morfeusz2-glueswi.cc to this directory,
   • execute:
   x86_64-w64-mingw32-g++ -D_REENTRANT -D__SWI_PROLOG__ -I./ -Iinclude/ -o morfeusz-swi.dll -shared morfeusz2-glueswi.cc  -static-libgcc -static-libstdc++ -Lbin/ -L./ -lswipl -lmorfeusz2

*/
#include <SWI-Prolog.h>

// The following line or the -D_GLIBCXX_USE_CXX11_ABI=0 compiler
// switch is needed with older versions of Morfeusz (before 2016)
// compiled with the old GCC ABI for std::string:
//
//#define _GLIBCXX_USE_CXX11_ABI 0

#include <string.h>
#include "morfeusz2.h"

using namespace morfeusz;

static functor_t F_interp;
static functor_t F_colon;
static functor_t F_MorphInterpretation;
static functor_t F_MorfeuszException;
static Morfeusz *m_instance;

//extern "C" 
static foreign_t pl_morfeusz_analyse(term_t st, term_t at) { 
  char *tekst;
  
  if ( PL_get_chars(st, &tekst,
		    CVT_ATOM|CVT_STRING|CVT_LIST|CVT_EXCEPTION|
		    BUF_DISCARDABLE|REP_UTF8) ) { 
    ResultsIterator *r=m_instance->analyse(tekst);

    term_t l = PL_copy_term_ref(at);
    term_t elem = PL_new_term_ref();

    while(r->hasNext()) {
      MorphInterpretation i=r->next(); 

      term_t t_lemma = PL_new_term_ref();
      size_t colon_pos = i.lemma.find_last_of(':');
      if (colon_pos == std::string::npos ||
	  colon_pos < 1 ||
	  colon_pos == i.lemma.length()-1 ) {
	// lemat nie zawiera dwukropka lub zawiera go na którymś końcu:
	if (!PL_unify_term(t_lemma,
			   PL_UTF8_CHARS, i.lemma.c_str()
			   )) PL_fail;
      } else {
	if (!PL_unify_term(t_lemma,
			   PL_FUNCTOR, F_colon,
			   PL_UTF8_CHARS, i.lemma.substr(0,colon_pos).c_str(),
			   PL_UTF8_CHARS, i.lemma.substr(colon_pos+1).c_str()
			   )) PL_fail;
      }
	
      term_t interp = PL_new_term_ref();
      if ( !PL_unify_list(l, elem, l) ||
	   //	   !PL_chars_to_term(i.lemma.c_str(), t_lemma) ||
	   !PL_chars_to_term(i.getTag(*m_instance).c_str(), interp) ||
	   // i.getLabelsAsString(*morfeusz)
	   !PL_unify_term(elem,
			  PL_FUNCTOR, F_interp,
			  PL_INT, i.startNode,
			  PL_INT, i.endNode,
			  PL_UTF8_CHARS, i.orth.c_str(),
			  //PL_UTF8_CHARS, i.lemma.c_str(),
			  PL_TERM, t_lemma,
			  PL_TERM, interp/* , */
			  /* PL_UTF8_CHARS, i.getName(*m_instance).c_str() */
			  )
	   ) PL_fail;
    }
    return PL_unify_nil(l);
  }
  PL_fail;
}

static foreign_t pl_dict_id(term_t namet) {
  std::string name = m_instance->getDictID();
  return PL_unify_atom_chars(namet, name.c_str());
}

static foreign_t unify_MorphInterpretation(MorphInterpretation &i, term_t t) {
  return PL_unify_term(t,
			PL_FUNCTOR,		F_MorphInterpretation,
			PL_UTF8_STRING,	i.orth.c_str(),
			PL_UTF8_STRING,	i.lemma.c_str(),
			PL_CHARS,		i.getTag(*m_instance).c_str(),
			PL_CHARS,		i.getName(*m_instance).c_str(),
			PL_CHARS,       i.getLabelsAsString(*m_instance).c_str());
}

static foreign_t unify_MorphInterpretations(std::vector<MorphInterpretation> &r,
                                            term_t t) {
  term_t tail = PL_copy_term_ref(t);
  term_t item = PL_new_term_ref();
  for (MorphInterpretation i : r) {
    if (!PL_unify_list(tail, item, tail) || !unify_MorphInterpretation(i, item))
      PL_fail;
  }
  return PL_unify_nil(tail);
}

static foreign_t pl_generate(term_t lemmat, term_t orths_t) {
  char *lemma;
  if (!PL_get_chars(lemmat, &lemma,
                    CVT_ATOM | CVT_STRING | CVT_LIST | CVT_EXCEPTION |
                        BUF_DISCARDABLE | REP_UTF8)) {
    PL_fail;
  }

  std::vector<MorphInterpretation> r;
  m_instance->generate(lemma, r);

  return unify_MorphInterpretations(r, orths_t);
}

static foreign_t pl_change_instance(term_t namet) {
  char *name;
  if (!PL_get_chars(namet, &name,
                    CVT_ATOM | CVT_STRING | CVT_LIST | CVT_EXCEPTION |
                        BUF_DISCARDABLE | REP_UTF8)) {
    PL_fail;
  }
  Morfeusz *m = NULL;
  try {
    m = Morfeusz::createInstance(name);
  } catch (morfeusz::MorfeuszException e) {
	term_t except;
	return ( (except = PL_new_term_ref()) &&
			 (PL_unify_term(except,
							PL_FUNCTOR,	F_MorfeuszException,
							PL_UTF8_STRING, e.what())) &&
			 (PL_raise_exception(except)) );
  }
  delete m_instance;
  m_instance = m;
  m_instance->setCharset(UTF8);
  PL_succeed;
}

extern "C" {
  install_t install() { 
    Morfeusz::dictionarySearchPaths.push_front(
        "/usr/share/morfeusz2/dictionaries");
    m_instance = Morfeusz::createInstance();
    m_instance->setCharset(UTF8);
    F_interp = PL_new_functor(PL_new_atom("i"), 5);
    F_colon = PL_new_functor(PL_new_atom(":"), 2);
    F_MorphInterpretation =
      PL_new_functor(PL_new_atom("morph_interpretation"), 5);
    F_MorfeuszException =
      PL_new_functor(PL_new_atom("morfeusz_exception"), 1);
    PL_register_foreign("morfeusz_analyse", 2, (pl_function_t)pl_morfeusz_analyse, 0);
    PL_register_foreign("dict_id", 1, (pl_function_t)pl_dict_id, 0);
    PL_register_foreign("change_instance", 1, (pl_function_t)pl_change_instance, 0);
    PL_register_foreign("generate", 2, (pl_function_t)pl_generate, 0);
  }
}

/* Local Variables: */
/* coding: utf-8 */
/* mode: c */
/* End: */
