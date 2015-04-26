#ifndef LEAPACTIONS_H
#define LEAPACTIONS_H
#include "Leap.h"
#include "math.h"
#include "Leap/DirectionDetector.h"
#include "Viewer/MouseControl.h"
#include "Viewer/CameraManipulator.h"

namespace Leap {

class LeapActions;

/**
 * @author Filip Mikle
 * @brief The Leap Action class
 * @brief All actions that will be executed on the leap sensor should be defined here and used in LeapListener
 */
class LeapActions
{
public:
	/**
	 * @brief Initializes MouseControl and sets zoomCounter to 0
	 */
	LeapActions();

	/**
	 * @brief Zooms the graph in or out depending on the hand movement(Circle gesture)
	 * @brief Clockwise - zooming out, Counterclockwise - zooming in
	 * @param gesture - CircleGesture to be used
	 */
	void Leap::LeapActions::zoomGraph( Gesture gesture );

	/**
	 * @brief Moves the graph using swipe gesture. Possible directions are UP, DOWN, LEFT, RIGHT
	 * @param gesture - SwipeGesture to be used
	 */
	void Leap::LeapActions::moveCamera( Gesture gesture );

	/**
	 * @brief Leap::LeapActions::rotateGraph
	 * @param gesture
	 */
	void Leap::LeapActions::rotateGraph( Gesture gesture );

	// These methods should not have names onSwipe, onXXX. They are placeholders for further functionality.
	// The names should correspond to the functionality provided by the method i.e. moveGraph();
	void Leap::LeapActions::onKeyTap( Gesture gesture );
	void Leap::LeapActions::onScreenTap( Gesture gesture );

	/**
	 * @brief Leap::LeapActions::changeViewAngle
	 * Change angle of camera in flymode using leap sensor
	 * @param direction - Direction enum from DirectionDetector.
	 */
	void Leap::LeapActions::changeViewAngle( Leap::DirectionDetector::Direction direction );
	/**
	 * @brief Leap::LeapActions::startMovingForward
	 * Starts the movement of camera forward.
	 */
	void Leap::LeapActions::startMovingForward();
	/**
	 * @brief Leap::LeapActions::stopMovingForward
	 * Stops forward movement of camera.
	 */
	void Leap::LeapActions::stopMovingForward();

	/**
	 * @brief isCameraMoving is variable to decide whether to move camera or rotate graph
	 */
	bool isCameraMoving;
private:
	/**
	 * @brief Controls mouse actions
	 */
	Vwr::MouseControl* mouse;
	Vwr::CameraManipulator* cmrManipulator;
};
}
#endif // LEAPACTIONS_H
