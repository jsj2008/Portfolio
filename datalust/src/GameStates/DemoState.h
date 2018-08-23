/*****************************************************************************/
/*!
  \file    DemoState.h
  \author  Zayd Gaudet
  \par     zayd.gaudet\@digipen.edu
  \par     09/07/2017

  \brief  
    
  All content © 2017 DigiPen (USA) Corporation, all rights reserved.
*/
/*****************************************************************************/
#ifndef DEMOSTATE_H
#define DEMOSTATE_H
#include <Util/Util.hpp>
#include <GameObject/ObjectFactory.h>
#include "GameState.h"

/*****************************************************************************/
/*!
    the game_state is the base class to be used as an interface for all other 
    gamestates
*/
/*****************************************************************************/
class DemoState : public GameState
{
  public:
    DemoState() = default;

    /*************************************************************************/
    /*!
        virtual destructor for the game_state class, must be implemented by each
        derived class
    */
    /*************************************************************************/
	~DemoState() = default;

    /*************************************************************************/
    /*!
        init function for the game_state class, must be implemented by each
        derived class
    */
    /*************************************************************************/
    void Init();

    /*************************************************************************/
    /*!
        update funcntion for the game_state class, must be implemented by each
        derived class

        \param float
            takes the time elapsed since the last update
    */
    /*************************************************************************/
    void Update(float dt);

    /*************************************************************************/
    /*!
        shutdown function for the game_state class, must be implemented by each
        derived class
    */
    /*************************************************************************/
    void Shutdown();
};

#endif //DEMOSTATE_H
