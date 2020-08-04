## Welcome to the Silvanus Box Generator Plugin for Autodesk's Fusion 360.

The latest alpha release of the plugin can be found [here](https://github.com/hobbyistmaker/silvanus/releases/tag/v0.5.3_alpha).

This plugin is currently developed in Python; although, an even more experimental version is being tested in C++, for much better performance.

### Current Features

* User definable length axis (X) non-removable dividers
* Override ability for panel thickness for all outside panels; inside panels will currently be the same thickness as defined by the main thickness input value.
* Enabling or disabling any of the outside panels (top, bottom, left, right, front, back)
* Automatically sized finger joints based on a user-defined "desired" value of finger-width.
* User specified kerf adjustment
* User-triggered preview capability while changing parameters
* Use of imperial and metric measurements
* Orientation of the box will be defined based on the default modeling orientation preference defined in Fusion.

### To-do before v1 (not a complete list)

* Add dividers along the width and height axes.
* Allow for manual definition of divider placement
* Allow for panel-specific kerf adjustment
* Allow creation of removable dividers
* Allow for different thicknesses of inside panels
* Allow user-selectable consistently sized finger joint sizes
* Allow for different finger joint sizes at each joint
* Create different patterns of finger joints instead of requiring all fingers to be equally sized along a joint (e.g. two small fingers, one large finger, two small fingers).

### Known Issues

* Finger sketches are not fully constrained, and may break if parameters are changed in the model after creation.
* Adding too many dividers to the box, so that the cumulative thickness of all of the dividers is larger than the length of the box, will cause a crash.
* Parameters are not yet used for dimensions, extrusions, etc. Updating parameter values in the User Parameters after creating a box will currently have no effect. This is currently under development.
* Panels and cut features will be sized appropriately when using kerf adjustment; however, if you check the placement of panels in the Fusion360 environment, the outside panels will be slightly off position; this has no impact to using the profiles for laser cutting, it is just cosmetic in the model. This will be fixed in a later release.
* Triggering a preview of the model requires the user to click on the Preview button at the bottom of the dialog. This was done to prevent major performance issues when changing parameter values in the dialog.
* Enabling or disabling the visibility of a panel or sketch while the dialog is open will crash Fusion 360. This appears to be a Fusion 360 bug, so the solution for this is unclear at the moment.
* Specifying a non-zero number of dividers in the dialog and then changing back to zero does not properly remove the dividers from the box. The dialog must be cancelled and re-run in order to resolve this issue for now.
* Some sketch names may be incorrectly defined when panels of different thickness exist along the same axis (e.g. a left panel of 3.2mm and a right panel of 5.8mm).
* Creating multiple boxes within the same project will cause conflicts when parameters are eventually enabled. For now, if you desire multiple boxes in 1 project, they must be created separately and then linked into another project.
* When in Y-up orientation within Fusion 360, the Front and Back panels are named incorrectly (i.e. the Front panel is named 'Back' and the Back panel is named 'Front')
* The add-in must be restarted after changing Fusion 360's distance display units in the Design general preferences (e.g. switching from metric to imperial units, or imperial to metric).
* Toggling a panel thickness override multiple times in one run can cause a plugin error to appear. This may or may not impact the results of clicking on "Create"

### Support or Contact

If you are using the plugin and have a problem, please [submit an issue](https://github.com/hobbyistmaker/silvanus/issues).
