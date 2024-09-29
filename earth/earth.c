/* This file is an image processing operation for GEGL
 *
 * GEGL is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * GEGL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright 2006 Øyvind Kolås <pippin@gimp.org>
 * 2024, beaver, Earth

optionally put graph here

simplex-noise scale=0.10 iterations=20 seed=4
component-extract component=cmyk-c
gradient-map color1=#2ebf2a color2=#2329f5
id=1 gimp:layer-mode layer-mode=hsv-value opacity=0.36 aux=[ 
 noise-solid x-size=2.9 y-size=2.9 seed=3333 detail=0  ]
id=2
over aux=[ ref=2 
noise-hsv holdness=4 hue-distance=13 saturation-distance=0 value-distance=0.43 opacity value=0.35 ] 
median-blur radius=0
gaussian-blur std-dev-x=0.7 std-dev-y=0.7
shadows-highlights 
shadows=-100
highlights=69.569999694824219
whitepoint=-0.91200000047683716
radius=3.2400000095367432
compress=50
shadows-ccorrect=0
highlights-ccorrect=0
id=3
gimp:layer-mode layer-mode=multiply opacity=0.37 aux=[ ref=3 emboss azimuth=30 elevation=41 depth=3 ]

]

end of syntax
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

/*Properties go here*/


property_seed    (landforms, _("Continent location"), rand)
    description  (_("Randomize the location of the Continents and Islands"))

property_double  (scale, _("Reverse Scaling"), 0.09)
    description  (_("The scale of the Continents and Islands"))
    value_range  (0.03, 0.2)

property_color  (earth, _("Earth color"), "#2ebf2a")
  description   (_("Color of the earth texture"))

property_color  (water, _("Water color"), "#2329f5")
  description   (_("Color of the water texture"))

property_seed    (lightseed, _("Light rotation"), rand2)
    description  (_("The random light rotation"))

property_int  (smooth, _("Smoothing"), 0)
    description  (_("The smoothing of the planet map"))
    value_range  (0, 6)

#else

#define GEGL_OP_META
#define GEGL_OP_NAME     earth
#define GEGL_OP_C_SOURCE earth.c

#include "gegl-op.h"

/*starred nodes go inside typedef struct */

typedef struct
{
 GeglNode *input;
 GeglNode *noise;
 GeglNode *ec;
 GeglNode *colors;
 GeglNode *lightshift;
 GeglNode *hsvvalue;
 GeglNode *string;
 GeglNode *nr;
 GeglNode *output;
}State;

static void attach (GeglOperation *operation)
{
  GeglNode *gegl = operation->node;
  GeglProperties *o = GEGL_PROPERTIES (operation);
  GeglColor *earth = gegl_color_new ("#2ebf2a");
  GeglColor *water = gegl_color_new ("#2329f5");


  State *state = o->user_data = g_malloc0 (sizeof (State));

/*new child node list is here, this is where starred nodes get defined

 state->newchildname = gegl_node_new_child (gegl, "operation", "lb:name", NULL);*/
  state->input    = gegl_node_get_input_proxy (gegl, "input");
  state->output   = gegl_node_get_output_proxy (gegl, "output");

  state->noise = gegl_node_new_child (gegl, "operation", "gegl:simplex-noise", "iterations", 20,  NULL);
/*ec's number is cmyk-c*/
  state->ec = gegl_node_new_child (gegl, "operation", "gegl:component-extract", "component", 10,  NULL);

  state->colors = gegl_node_new_child (gegl, "operation", "port:gradient-map", "color1", earth, "color2", water,  NULL); 

  state->lightshift = gegl_node_new_child (gegl, "operation", "gegl:noise-solid", "x-size", 2.9, "y-size", 2.9,  "detail", 0,  NULL);

 state->hsvvalue = gegl_node_new_child (gegl, "operation", "gimp:layer-mode", "layer-mode", 40, "opacity", 0.36, NULL); 

  state->nr = gegl_node_new_child (gegl, "operation", "gegl:noise-reduction",  NULL);


#define LONG \
" id=2 over aux=[ ref=2 noise-hsv holdness=4 hue-distance=13 saturation-distance=0 value-distance=0.43 opacity value=0.35 ] median-blur radius=0 gaussian-blur std-dev-x=0.7 std-dev-y=0.7 shadows-highlights shadows=-100 highlights=69.569999694824219 whitepoint=-0.91200000047683716 radius=3.2400000095367432 compress=50 shadows-ccorrect=0 highlights-ccorrect=0 id=3 gimp:layer-mode layer-mode=multiply opacity=0.37 aux=[ ref=3 emboss azimuth=30 elevation=41 depth=3 ] ]  "\

 state->string = gegl_node_new_child (gegl, "operation", "gegl:gegl", "string", LONG,  NULL); 

/*meta redirect property to new child orders go here

 gegl_operation_meta_redirect (operation, "propertyname", state->newchildname,  "originalpropertyname");
*/
 gegl_operation_meta_redirect (operation, "scale", state->noise,  "scale");
 gegl_operation_meta_redirect (operation, "landforms", state->noise,  "seed");
 gegl_operation_meta_redirect (operation, "earth", state->colors,  "color1");
 gegl_operation_meta_redirect (operation, "water", state->colors,  "color2");
 gegl_operation_meta_redirect (operation, "lightseed", state->lightshift,  "seed");
 gegl_operation_meta_redirect (operation, "smooth", state->nr,  "iterations");

}

static void
update_graph (GeglOperation *operation)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  State *state = o->user_data;
  if (!state) return;

  gegl_node_link_many (state->noise, state->ec, state->colors, state->hsvvalue, state->string, state->nr, state->output,  NULL);
  gegl_node_connect (state->hsvvalue, "aux", state->lightshift, "output"); 
/*optional connect from and too is here
  gegl_node_connect (state->blendmode, "aux", state->lastnodeinlist, "output"); */

}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;
GeglOperationMetaClass *operation_meta_class = GEGL_OPERATION_META_CLASS (klass);
  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;
  operation_meta_class->update = update_graph;

  gegl_operation_class_set_keys (operation_class,
    "name",        "lb:earth",
    "title",       _("Earth texture"),
    "reference-hash", "heavenisaplaceonearth",
    "description", _("Texture of earth-like Continents and Islands"),
/*<Image>/Colors <Image>/Filters are top level menus in GIMP*/
    "gimp:menu-path", "<Image>/Filters/Render/Fun",
    "gimp:menu-label", _("Earth texture..."),
    NULL);
}

#endif
