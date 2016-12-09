
/*
  AnimatedGifSaver! 
  by Marco Tarini
  
  A simple class to define simple looped animations
  and save them as gif.
  
  Usage: 
  Just include this file in your code and use the class. 
  Does not need to include giflib.
  Include gifSave.cpp in your project and link with -lgif
  
  This is a quick and dirty class. 
  At any given time, at most one instance of AnimatedGifSaver must exist!
   
*/

typedef unsigned char Byte;

class AnimatedGifSaver{
public:
  
  // Constructor. Use a new instance for each new animation you want to save.
  // All the frames will be sized [sx]X[sy]
  AnimatedGifSaver(int sx, int sy);

  // Descrutcor
  ~AnimatedGifSaver();
  
  // Adds a frame that is to last [dt] seconds
  // "data" is the image (r,g,b,r,g,b,r,g,b...), bottom-to-top
  // The first frame defines the palette
  bool AddFrame(Byte* data,  float dt);
  
  // Saves the results in a gif file
  bool Save(const char* filename);
  
};
