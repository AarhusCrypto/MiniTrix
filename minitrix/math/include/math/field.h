#ifndef FIELD_H
#define FIELD_H

/*
 Field
{
 Field F = GF8_new();
 Fe one = F->one();
 Fe two = 0;
 if (F->add(&two,one,one) != 0)  {
   // ERROR 
 }
 
 MATRIX m = Matrix_new(F);
 
 
  This idea is good but blows up by a factor of eight for GF8. We'll
 stick to polynomial.h for now !
 
 */
typedef void * Fe;

typedef struct _field_ {
  /*!
   * Compute {a}+{b} and store the result in {r}
   */
  int (*add)(Fe * r, Fe a, Fe b);

  /*!
   * Compute {a}*{b} and store the result in {r}
   */
  int (*mul)(Fe * r, Fe a, Fe b);

  /*!
   * Compute {a}/{b} and store the result in {r}
   */
  int (*div)(Fe * r, Fe a, Fe b);

  /*!
   * Compute {a}-{b} and store the result in {r}
   */ 
  int (*sub)(Fe * r, Fe a, Fe b);

  /*!
   * Compute {a}^{b} and store the result in {r}
   */
  int (*exp)(Fe * r, Fe a, Fe b);

  /*
   * The multiplicative identity
   */
  Fe (*one)(void);

  /*!
   * The additive identity
   */
  Fe (*zero)(void);
} * Field;

#endif
