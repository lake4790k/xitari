//============================================================================
//
//   SSSS    tt          lll  lll       
//  SS  SS   tt           ll   ll        
//  SS     tttttt  eeee   ll   ll   aaaa 
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
//
// See the file "license" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: TIA.hxx,v 1.42 2007/02/22 02:15:46 stephena Exp $
//============================================================================

#ifndef TIA_HXX
#define TIA_HXX

namespace ale {

class Console;
class System;
class Serializer;
class Deserializer;
class Settings;

} // namespace ale

#include "m6502/src/bspf/src/bspf.hxx"
#include "Sound.hxx"
#include "m6502/src/Device.hxx"
#include "MediaSrc.hxx"

namespace ale {

typedef unsigned long long uint64_t;

/**
  This class is a device that emulates the Television Interface Adapator 
  found in the Atari 2600 and 7800 consoles.  The Television Interface 
  Adapator is an integrated circuit designed to interface between an 
  eight bit microprocessor and a television video modulator. It converts 
  eight bit parallel data into serial outputs for the color, luminosity, 
  and composite sync required by a video modulator.  

  This class outputs the serial data into a frame buffer which can then
  be displayed on screen.

  @author  Bradford W. Mott
  @version $Id: TIA.hxx,v 1.42 2007/02/22 02:15:46 stephena Exp $
*/
class TIA : public Device , public MediaSource
{
  public:
    friend class TIADebug;

    /**
      Create a new TIA for the specified console

      @param console  The console the TIA is associated with
      @param settings The settings object for this TIA device
    */
    TIA(const Console& console, Settings& settings);
 
    /**
      Destructor
    */
    virtual ~TIA();

  public:
    /**
      Get a null terminated string which is the device's name (i.e. "M6532")

      @return The name of the device
    */
    virtual const char* name() const;

    /**
      Reset device to its power-on state
    */
    virtual void reset();

    /**
      Reset frame to change XStart/YStart/Width/Height properties
    */
    virtual void frameReset();

    /**
      Notification method invoked by the system right before the
      system resets its cycle counter to zero.  It may be necessary
      to override this method for devices that remember cycle counts.
    */
    virtual void systemCyclesReset();

    /**
      Install TIA in the specified system.  Invoked by the system
      when the TIA is attached to it.

      @param system The system the device should install itself in
    */
    virtual void install(System& system);

    /**
      Saves the current state of this device to the given Serializer.

      @param out The serializer device to save to.
      @return The result of the save.  True on success, false on failure.
    */
    virtual bool save(Serializer& out);

    /**
      Loads the current state of this device from the given Deserializer.

      @param in The deserializer device to load from.
      @return The result of the load.  True on success, false on failure.
    */
    virtual bool load(Deserializer& in);

  public:
    /**
      Get the byte at the specified address

      @return The byte at the specified address
    */
    virtual uInt8 peek(uInt16 address);

    /**
      Change the byte at the specified address to the given value

      @param address The address where the value should be stored
      @param value The value to be stored at the address
    */
    virtual void poke(uInt16 address, uInt8 value);

  public:
    /**
      This method should be called at an interval corresponding to
      the desired frame rate to update the media source.
    */
    virtual void update();

    /**
      Answers the current frame buffer

      @return Pointer to the current frame buffer
    */
    uInt8* currentFrameBuffer() const { return myCurrentFrameBuffer; }

    /**
      Answers the previous frame buffer

      @return Pointer to the previous frame buffer
    */
    uInt8* previousFrameBuffer() const { return myPreviousFrameBuffer; }

    /**
      Answers the height of the frame buffer

      @return The frame's height
    */
    uInt32 height() const;

    /**
      Answers the width of the frame buffer

      @return The frame's width
    */
    uInt32 width() const;

    /**
      Answers the total number of scanlines the media source generated
      in producing the current frame buffer. For partial frames, this
      will be the current scanline.

      @return The total number of scanlines generated
    */
    uInt32 scanlines() const;

    /**
      Answers the current color clock we've gotten to on this scanline.

      @return The current color clock
    */
    uInt32 clocksThisLine() const;

    /**
      Sets the sound device for the TIA.
    */
    void setSound(Sound& sound);

    enum TIABit {
      P0,   // Descriptor for Player 0 Bit
      P1,   // Descriptor for Player 1 Bit
      M0,   // Descriptor for Missle 0 Bit
      M1,   // Descriptor for Missle 1 Bit
      BL,   // Descriptor for Ball Bit
      PF    // Descriptor for Playfield Bit
    };

    /**
      Enables/disables the specified TIA bit.

      @return  Whether the bit was enabled or disabled
    */
    bool enableBit(TIABit b, bool mode) { myBitEnabled[b] = mode; return mode; }

    /**
      Toggles the specified TIA bit.

      @return  Whether the bit was enabled or disabled
    */
    bool toggleBit(TIABit b) { myBitEnabled[b] = !myBitEnabled[b]; return myBitEnabled[b]; }

    /**
      Enables/disables all TIABit bits.

      @param mode  Whether to enable or disable all bits
    */
    void enableBits(bool mode) { for(uInt8 i = 0; i < 6; ++i) myBitEnabled[i] = mode; }

  private:
    // Compute the ball mask table
    void computeBallMaskTable();

    // Compute the collision decode table
    void computeCollisionTable();

    // Compute the missle mask table
    void computeMissleMaskTable();

    // Compute the player mask table
    void computePlayerMaskTable();

    // Compute the player position reset when table
    void computePlayerPositionResetWhenTable();

    // Compute the player reflect table
    void computePlayerReflectTable();

    // Compute playfield mask table
    void computePlayfieldMaskTable();

  private:
    // Update the current frame buffer up to one scanline
    void updateFrameScanline(uInt32 clocksToUpdate, uInt32 hpos);
   
    // Update the current frame buffer to the specified color clock
    void updateFrame(Int32 clock);

    // Waste cycles until the current scanline is finished
    void waitHorizontalSync();

    // Grey out current framebuffer from current scanline to bottom
    void greyOutFrame();

    // Clear both internal TIA buffers to black (palette color 0)
    void clearBuffers();

    // Set up bookkeeping for the next frame
    void startFrame();

    // Update bookkeeping at end of frame
    void endFrame();

  private:
    // Console the TIA is associated with
    const Console& myConsole;

    // Settings object the TIA is associated with
    const Settings& mySettings;

    // Sound object the TIA is associated with
    Sound* mySound;

  private:
    // Indicates if color loss should be enabled or disabled.  Color loss
    // occurs on PAL (and maybe SECAM) systems when the previous frame
    // contains an odd number of scanlines.
    bool myColorLossEnabled;

    // Indicates whether we're done with the current frame. poke() clears this
    // when VSYNC is strobed or the max scanlines/frame limit is hit.
    bool myPartialFrameFlag;

  private:
    // Number of frames displayed by this TIA
    int myFrameCounter;

    // Pointer to the current frame buffer
    uInt8* myCurrentFrameBuffer;

    // Pointer to the previous frame buffer
    uInt8* myPreviousFrameBuffer;

    // Pointer to the next pixel that will be drawn in the current frame buffer
    uInt8* myFramePointer;

    // Indicates where the scanline should start being displayed
    uInt32 myFrameXStart;

    // Indicates the width of the scanline 
    uInt32 myFrameWidth;

    // Indicated what scanline the frame should start being drawn at
    uInt32 myFrameYStart;

    // Indicates the height of the frame in scanlines
    uInt32 myFrameHeight;

  private:
    // Indicates offset in scanlines when display should begin
     // (aka the Display.YStart property)
    uInt32 myYStart;

     // Height of display (aka Display.Height)
    uInt32 myHeight;

    // Indicates offset in color clocks when display should begin
    uInt32 myStartDisplayOffset;

    // Indicates offset in color clocks when display should stop
    uInt32 myStopDisplayOffset;

  private:
    // Indicates color clocks when the current frame began
    Int32 myClockWhenFrameStarted;

    // Indicates color clocks when frame should begin to be drawn
    Int32 myClockStartDisplay;

    // Indicates color clocks when frame should stop being drawn
    Int32 myClockStopDisplay;

    // Indicates color clocks when the frame was last updated
    Int32 myClockAtLastUpdate;

    // Indicates how many color clocks remain until the end of 
    // current scanline.  This value is valid during the 
    // displayed portion of the frame.
    Int32 myClocksToEndOfScanLine;

    // Indicates the total number of scanlines generated by the last frame
    Int32 myScanlineCountForLastFrame;

    // Indicates the current scanline during a partial frame.
    Int32 myCurrentScanline;

    // Indicates the maximum number of scanlines to be generated for a frame
    Int32 myMaximumNumberOfScanlines;

  private:
    // Color clock when VSYNC ending causes a new frame to be started
    Int32 myVSYNCFinishClock; 

  private:
    enum
    {
      myP0Bit = 0x01,         // Bit for Player 0
      myM0Bit = 0x02,         // Bit for Missle 0
      myP1Bit = 0x04,         // Bit for Player 1
      myM1Bit = 0x08,         // Bit for Missle 1
      myBLBit = 0x10,         // Bit for Ball
      myPFBit = 0x20,         // Bit for Playfield
      ScoreBit = 0x40,        // Bit for Playfield score mode
      PriorityBit = 0x080     // Bit for Playfield priority
    };

    // Bitmap of the objects that should be considered while drawing
    uInt8 myEnabledObjects;

  private:
    uInt8 myVSYNC;        // Holds the VSYNC register value
    uInt8 myVBLANK;       // Holds the VBLANK register value

    uInt8 myNUSIZ0;       // Number and size of player 0 and missle 0
    uInt8 myNUSIZ1;       // Number and size of player 1 and missle 1

    uInt8 myPlayfieldPriorityAndScore;
    uInt32 myColor[4];
    uInt8 myPriorityEncoder[2][256];

    uInt32& myCOLUBK;       // Background color register (replicated 4 times)
    uInt32& myCOLUPF;       // Playfield color register (replicated 4 times)
    uInt32& myCOLUP0;       // Player 0 color register (replicated 4 times)
    uInt32& myCOLUP1;       // Player 1 color register (replicated 4 times)

    uInt8 myCTRLPF;       // Playfield control register

    bool myREFP0;         // Indicates if player 0 is being reflected
    bool myREFP1;         // Indicates if player 1 is being reflected

    uInt32 myPF;          // Playfield graphics (19-12:PF2 11-4:PF1 3-0:PF0)

    uInt8 myGRP0;         // Player 0 graphics register
    uInt8 myGRP1;         // Player 1 graphics register
    
    uInt8 myDGRP0;        // Player 0 delayed graphics register
    uInt8 myDGRP1;        // Player 1 delayed graphics register

    bool myENAM0;         // Indicates if missle 0 is enabled
    bool myENAM1;         // Indicates if missle 0 is enabled

    bool myENABL;         // Indicates if the ball is enabled
    bool myDENABL;        // Indicates if the virtically delayed ball is enabled

    Int8 myHMP0;          // Player 0 horizontal motion register
    Int8 myHMP1;          // Player 1 horizontal motion register
    Int8 myHMM0;          // Missle 0 horizontal motion register
    Int8 myHMM1;          // Missle 1 horizontal motion register
    Int8 myHMBL;          // Ball horizontal motion register

    bool myVDELP0;        // Indicates if player 0 is being virtically delayed
    bool myVDELP1;        // Indicates if player 1 is being virtically delayed
    bool myVDELBL;        // Indicates if the ball is being virtically delayed

    bool myRESMP0;        // Indicates if missle 0 is reset to player 0
    bool myRESMP1;        // Indicates if missle 1 is reset to player 1

    uInt16 myCollision;    // Collision register

    // Note that these position registers contain the color clock 
    // on which the object's serial output should begin (0 to 159)
    Int16 myPOSP0;         // Player 0 position register
    Int16 myPOSP1;         // Player 1 position register
    Int16 myPOSM0;         // Missle 0 position register
    Int16 myPOSM1;         // Missle 1 position register
    Int16 myPOSBL;         // Ball position register

  private:
    // Graphics for Player 0 that should be displayed.  This will be
    // reflected if the player is being reflected.
    uInt8 myCurrentGRP0;

    // Graphics for Player 1 that should be displayed.  This will be
    // reflected if the player is being reflected.
    uInt8 myCurrentGRP1;

    // It's VERY important that the BL, M0, M1, P0 and P1 current
    // mask pointers are always on a uInt32 boundary.  Otherwise,
    // the TIA code will fail on a good number of CPUs.

    // Pointer to the currently active mask array for the ball
    uInt8* myCurrentBLMask;

    // Pointer to the currently active mask array for missle 0
    uInt8* myCurrentM0Mask;

    // Pointer to the currently active mask array for missle 1
    uInt8* myCurrentM1Mask;

    // Pointer to the currently active mask array for player 0
    uInt8* myCurrentP0Mask;

    // Pointer to the currently active mask array for player 1
    uInt8* myCurrentP1Mask;

    // Pointer to the currently active mask array for the playfield
    uInt32* myCurrentPFMask;

  private:
    // Indicates when the dump for paddles was last set
    Int32 myDumpDisabledCycle;

    // Indicates if the dump is current enabled for the paddles
    bool myDumpEnabled;

  private:
    // Color clock when last HMOVE occured
    Int32 myLastHMOVEClock;

    // Indicates if HMOVE blanks are currently enabled
    bool myHMOVEBlankEnabled;

    // Indicates if we're allowing HMOVE blanks to be enabled
    bool myAllowHMOVEBlanks;

    // TIA M0 "bug" used for stars in Cosmic Ark flag
    bool myM0CosmicArkMotionEnabled;

    // Counter used for TIA M0 "bug" 
    uInt32 myM0CosmicArkCounter;

    // Answers whether specified bits (from TIABit) are enabled or disabled
    bool myBitEnabled[6];

     // Has current frame been "greyed out" (has updateScanline() been run?)
     bool myFrameGreyed;

  private:
    // Ball mask table (entries are true or false)
    uInt8 ourBallMaskTable[4][4][512];

    // Used to set the collision register to the correct value
    uInt16 ourCollisionTable[64];

    // A mask table which can be used when an object is disabled
    uInt8 ourDisabledMaskTable[1024];

    // Indicates the update delay associated with poking at a TIA address
    const Int16 ourPokeDelayTable[64] = {
   0,  1,  0,  0,  8,  8,  0,  0,  0,  0,  0,  1,  1, -1, -1, -1,
   0,  0,  8,  8,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};;

    // Missle mask table (entries are true or false)
    uInt8 ourMissleMaskTable[4][8][4][512];

    // Used to convert value written in a motion register into 
    // its internal representation
    const Int32 ourCompleteMotionTable[128][16] = {
  { 0, -1, -2, -3, -4, -5, -6, -7,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -6, -7,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -6, -7,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -6, -7,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -6, -6,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -5, -5,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -5, -5, -5,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -4, -4, -4, -4,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -3, -3, -3, -3, -3,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -2, -2, -2, -2, -2,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -2, -2, -2, -2, -2, -2,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0, -1, -1, -1, -1, -1, -1, -1,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 0,  0,  0,  0,  0,  0,  0,  0,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 1,  1,  1,  1,  1,  1,  1,  1,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 1,  1,  1,  1,  1,  1,  1,  1,  8,  7,  6,  5,  4,  3,  2,  1}, // HBLANK
  { 2,  2,  2,  2,  2,  2,  2,  2,  8,  7,  6,  5,  4,  3,  2,  2}, // HBLANK
  { 3,  3,  3,  3,  3,  3,  3,  3,  8,  7,  6,  5,  4,  3,  3,  3}, // HBLANK
  { 4,  4,  4,  4,  4,  4,  4,  4,  8,  7,  6,  5,  4,  4,  4,  4}, // HBLANK
  { 4,  4,  4,  4,  4,  4,  4,  4,  8,  7,  6,  5,  4,  4,  4,  4}, // HBLANK
  { 5,  5,  5,  5,  5,  5,  5,  5,  8,  7,  6,  5,  5,  5,  5,  5}, // HBLANK
  { 6,  6,  6,  6,  6,  6,  6,  6,  8,  7,  6,  6,  6,  6,  6,  6}, // HBLANK
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0, -1, -2,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0, -1, -2, -3,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0, -1, -2, -3,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0, -1, -2, -3, -4,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0, -1, -2, -3, -4, -5,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0, -1, -2, -3, -4, -5, -6,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0, -1, -2, -3, -4, -5, -6,  0,  0,  0,  0,  0,  0,  0,  0},
  { 0, -1, -2, -3, -4, -5, -6, -7,  0,  0,  0,  0,  0,  0,  0,  0},
  {-1, -2, -3, -4, -5, -6, -7, -8,  0,  0,  0,  0,  0,  0,  0,  0},
  {-2, -3, -4, -5, -6, -7, -8, -9,  0,  0,  0,  0,  0,  0,  0, -1},
  {-2, -3, -4, -5, -6, -7, -8, -9,  0,  0,  0,  0,  0,  0,  0, -1},
  {-3, -4, -5, -6, -7, -8, -9,-10,  0,  0,  0,  0,  0,  0, -1, -2},
  {-4, -5, -6, -7, -8, -9,-10,-11,  0,  0,  0,  0,  0, -1, -2, -3},
  {-5, -6, -7, -8, -9,-10,-11,-12,  0,  0,  0,  0, -1, -2, -3, -4},
  {-5, -6, -7, -8, -9,-10,-11,-12,  0,  0,  0,  0, -1, -2, -3, -4},
  {-6, -7, -8, -9,-10,-11,-12,-13,  0,  0,  0, -1, -2, -3, -4, -5},
  {-7, -8, -9,-10,-11,-12,-13,-14,  0,  0, -1, -2, -3, -4, -5, -6},
  {-8, -9,-10,-11,-12,-13,-14,-15,  0, -1, -2, -3, -4, -5, -6, -7},
  {-8, -9,-10,-11,-12,-13,-14,-15,  0, -1, -2, -3, -4, -5, -6, -7},
  { 0, -1, -2, -3, -4, -5, -6, -7,  8,  7,  6,  5,  4,  3,  2,  1}  // HBLANK
};

    // Indicates if HMOVE blanks should occur for the corresponding cycle
    const bool ourHMOVEBlankEnableCycles[128] = {
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,   // 00
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,   // 10
  true,  false, false, false, false, false, false, false, false, false,  // 20
  false, false, false, false, false, false, false, false, false, false,  // 30
  false, false, false, false, false, false, false, false, false, false,  // 40
  false, false, false, false, false, false, false, false, false, false,  // 50
  false, false, false, false, false, false, false, false, false, false,  // 60
  false, false, false, false, false, true                                // 70
};

    // Player mask table
    uInt8 ourPlayerMaskTable[4][2][8][512];

    // Indicates if player is being reset during delay, display or other times
    Int8 ourPlayerPositionResetWhenTable[8][256][256];

    // Used to reflect a players graphics
    uInt8 ourPlayerReflectTable[256];

    // Playfield mask table for reflected and non-reflected playfields
    uInt32 ourPlayfieldTable[2][256];

  private:
    // Copy constructor isn't supported by this class so make it private
    TIA(const TIA&);

    // Assignment operator isn't supported by this class so make it private
    TIA& operator = (const TIA&);

  /** ALE-specific */
  private:
    bool fastUpdate;
   
    // Updates the frame's scanline but not the frame buffer 
    void updateFrameScanlineFast(uInt32 clocksToUpdate, uInt32 hpos);

    /** Some new functions for speed-up.*/
    uInt8 INPT0_3(const uInt8 &noise, const Int32 &r);
    
};

} // namespace ale

#endif
