/*!
 * \file Parsing command line arguments into a map is the tast of this
 * component.
 *
 *
 */
#ifndef OPTIONS_H
#define OPTIONS_H

#include <map.h>

/*!
 * Given a list of strings each either on the form -<option name> or
 * -<option name> <option value> a map is created such that <option
 * name>'s are keys mapping to the corresponding <option values>.
 */
Map Options_New(OE oe, int argc, char **args);

/*!
 * Destroy the map return by {Options_New}.
 */
void Options_Destroy(Map * m);

#endif
