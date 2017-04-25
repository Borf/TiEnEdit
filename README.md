# TiEnEdit
An editor for TiEn scenes

This tool can open json-serialized TiEn scenes, change the nodes / components, and serialize it back to disk.  
TiEn renders based on a [scenegraph](https://en.wikipedia.org/wiki/Scene_graph). 
The scene is built up out of nodes, that have a number of components attached to them to specify their behaviour. 
All nodes should have a translate component. 

## Window:
The window consists of a couple of subpanels:
- Scene overview  
  Shows a treeview of all the nodes in your scene.
- Properties panel  
  Shows the properties of the currently selected nodes. If more than one node is selected, behaviour is still undefined ;)
- Asset browser  
  Allows you to browse through the filesystem to add assets to your scene.
- 3D view  
  Shows a 3d preview of your scene, using TiEn rendering. Extra debug rendering options are available through the menu

## Camera controls:
The middle mouse button is mainly used for camera control. When the middle mouse button is pressed, use W,A,S,D for FPS controls. Use Q and Z to fly up and down. Hold shift to move the camera faster

## Selection controls:
### 3D view
Objects are selectable in the 3D view using the left mouse button. 
By holding shift, other objects can be added to your selection.
Objects are selected using a raycast on the polygons, even on backfaces. This means you need to move the camera inside of objects, if you have objects inside of other objects that you want to select

### Scene overview
Objects can be selected in the scene overview by clicking them.
By holding Control, other objects can be added to your selection.
By holding Shift, all the objects between the currently selected object and the new selected object will be selected

## Object manipulation
Selected objects in the 3d view can be moved, scaled and rotated. 
- G to grab objects
- GG to grab objects, without moving the child-nodes of this node
- R to rotate objects around a central axis
- RR to rotate objects around their own axis
- S to scale objects

Press X,Y or Z to only translate/rotate/scale on a certain axis.  
Press Shift+X,Y or Z to only transform on the plane (Shift+X will translate on the Y,X plane)  
Hold shift during transforming to snap the values to a grid

Objects can be copy/pasted using Ctrl+c and Ctrl+v. Ctrl+z can be used to undo some actions


## Components
Components can be added to nodes to manipulate what nodes do. 
It's also possible (and encouraged) to create your own nodes in code, to manipulate object behaviour. 
There are a number of default components (that will be increased over time).  
Some components have texture or model property boxes, where assets from the asset browser can be dragged upon.  
Floating point fields can easily be adjusted by dragging them with your right mouse button

- **Transform**  
  Transforms the node with a local translation, rotation and scale.  
  Transforms continue recursive on parent transforms
  - Local translate: a local translation (x, y, z)
  - Local scale: a local scale value (x,y,z)
  - Local Rotate: a local rotation (yaw,pitch,roll). These values are stored internally as a quaternion, which could cause some confusion / problems
- **ModelRenderer** 
  Renders a model at the transform of this node.
  - Filename: the filename of the model being rendered. Can drag a model from the asset browser in here
  - Casts Shadows: should this object cast shadows. Defaults to true
  - Cull backfaces: Should the backfaces of this model be drawn. Defaults to true
  - Materials: shows the materials used for this model. Materials can be manipulated per node
- **AnimatedModelRenderer***  
  Renders a model, with an animation, at the transform of this node. 
  - Filename: the filename of the model being rendered. Can drag a model from the asset browser in here
  - Casts Shadows: should this object cast shadows. Defaults to true
  - Cull backfaces: Should the backfaces of this model be drawn. Defaults to true
  - Materials: shows the materials used for this model. Materials can be manipulated per node
- **MeshRenderer**  
  Renders a custom mesh at the transform of this node
  - Casts Shadows: should this object cast shadows. Defaults to true
  - Cull backfaces: Should the backfaces of this model be drawn. Defaults to true
  - Materials: shows the materials used for this model. Materials can be manipulated per node
- **TerrainRenderer**  

- **Light**  
  A virtual light source. 
  - Color: the color of this light source
  - Intensity: the intensity of this light
  - Light type: the type of light; directional, pointlight or spotlight
  - Spotlight angle: the angle of the light if this light is a spotlight
  - Range: the range of the light. Not used for directional lights
  - Ambient: the ambient factor of this light. Only for directional lights
  - Cutoff: the exponent of the exponential falloff gradient. Should be beteen 0 and 1
  - Baking: Should shadows be calculated once or every frame
  - Shadow: what kind of shadow should this light cast. Note: shadowvolume is not implemented yet
- **Camera**  
  A virtual camera. Every scene should contain at least one camera, this is where the scene can be rendered from.
  A camera can also have different components that work together with the camera component, like a skybox component or a postprocess component
  - no properties.
- **DynamicSkyBox**  
  Renders a dynamic skybox behind the scene. This skybox has a dynamic sky color, based on the time of day, and also renders a sun and a moon
  - Time: The time of day, as a floating point 0-24. 12.5 means 12:30
- **StaticSkyBox**  
  Renders a static skybox behind the scene. The skybox uses 6 textures to render
- **RigidBody**  
  Adds physics to this object. By adding this to an object, a physx actor gets added to the phyx world
  - Mass: the mass of this body, in kg
  - Type: what kind of object is this. Static for non-moving objects, dynamic for moving objects
- **BoxCollider**  
  Defines the shape of the physics object as a box
  - size: the size of the collider
  - offset: used for translating the collider relative to the origin (for models that are not centered)
- **MeshCollider**  
  Defines the shape of the physics object based on a mesh or model attached to this node.
  - Convex: should the collider be a convex hull or a triangle mesh. Convex hulls are more efficient and can be used for dynamic objects, non-convex triangle meshes can't be used for dynamic objects
- **SphereCollider**  
  Defines the shape of the physics object as a sphere
  - size: the size of the collider
  - offset: used for translating the collider relative to the origin (for models that are not centered)
- **TerrainCollider**  
