// Music 256a / CS 476a | fall 2016
// Assignment 2
// CCRMA, Stanford University
//
// Author: Mark Rau
//
// A simple guitar chord player. Keyboard keys a, b, c, d, e, f, g, will determine which chord is played. 
// They are set to G major so the chords are Em, G, Am, Bm, C, and D. 
// A slider is used to pluck the strings which are synthesized using a Karplus-Strong model. 
// Note: The guitar is "tuned" to have a capo on the third fret.
// This was done because the string model didn't allow for notes lower than Fs/512. 