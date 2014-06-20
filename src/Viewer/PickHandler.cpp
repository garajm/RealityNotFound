#include "Viewer/PickHandler.h"

#include "Viewer/DataHelper.h"
#include "Viewer/CoreGraph.h"
#include "Viewer/CameraManipulator.h"

#include "Util/ElementSelector.h"

#include "Manager/Manager.h"
#include "Network/Client.h"
#include "Network/Server.h"

#include "Core/Core.h"
#include "Layout/LayoutThread.h"
#include "Layout/Shape_Cube.h"

#include "Util/ApplicationConfig.h"

#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/BlendFunc>
#include <osg/ValueObject>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"

using namespace Vwr;

PickHandler::PickHandler(Vwr::CameraManipulator * cameraManipulator, Vwr::CoreGraph * coreGraph)
{
	//vytvorenie timera a vynulovanie premennych
	timer = new QTimer();
	connect(timer,SIGNAL(timeout()), this, SLOT(mouseTimerTimeout()));
	eaPush = NULL;
	eaRel = NULL;
	aaPush = NULL;
	aaRel = NULL;

	this->cameraManipulator = cameraManipulator;
	this->coreGraph = coreGraph;

	this->appConf = Util::ApplicationConfig::get();

	_mX = _mY = 0;
	isCtrlPressed = false;
	isShiftPressed = false;
	isAltPressed = false;
    isXPressed = false;
    isYPressed = false;
    isZPressed = false;
	isDrawingSelectionQuad = false;
	isDragging = false;
	isManipulatingNodes = false;
	pickMode = PickMode::NONE;
	selectionType = SelectionType::ALL;
}

bool PickHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
	switch( ea.getEventType() )
	{
	case osgGA::GUIEventAdapter::PUSH:
	{
		//vynulovanie premennej pre ulozenie release eventu
		eaRel = NULL;
		// ak dojde push a nie je zapnuty timer tak spusti sa timer a ulozia eventy
		if (!timer->isActive())
		{
			Util::ApplicationConfig * appConf = Util::ApplicationConfig::get();
			timer->start(appConf->getValue("Mouse.DoubleClickTime").toInt());
			eaPush = &ea;
			aaPush = &aa;
			return false;
		}
		return handlePush(ea, aa);
	}
	case osgGA::GUIEventAdapter::DOUBLECLICK:
	{
		//ak dosiel double click tak vypnut timer
		timer->stop();
		eaRel = NULL;
		return handleDoubleclick(ea, aa);
	}
	case osgGA::GUIEventAdapter::MOVE:
	{
		return handleMove(ea, aa);
	}
	case osgGA::GUIEventAdapter::DRAG:
	{
		Network::Client * client = Network::Client::getInstance();
		if (client->isConnected()){
			client -> sendMovedNodesPosition();
		}
		//ak je drag a ide timer tak vypnut timer a vyvolat push
		//zaruci sa tak spravne spracovany drag
		if (timer->isActive())
		{
			timer->stop();
			handlePush(*eaPush, *aaPush);
		}
		//uz kvoli dalsiemu pokracovaniu dragu
		return handleDrag(ea, aa);
	}
	case osgGA::GUIEventAdapter::RELEASE:
	{
		Network::Server * server = Network::Server::getInstance();
		if (server->isListening()) {
			server -> sendMoveNodes();
		}
		//ak je release a je timer aktivny tak sa ulozi event a nevyvola sa
		if (timer->isActive())
		{
			eaRel = &ea;
			aaRel = &aa;
			return false;
		}
		return handleRelease(ea, aa);
	}
	case osgGA::GUIEventAdapter::KEYDOWN:
	{
		return handleKeyDown(ea, aa);
	}
	case::osgGA::GUIEventAdapter::KEYUP:
	{
		return handleKeyUp(ea, aa);
	}
    case::osgGA::GUIEventAdapter::SCROLL:
    {
        return handleScroll(ea, aa);
    }
	default:

		return false;
	}
}

void PickHandler::mouseTimerTimeout()
{
	//ak dobehne timer tak pouzivatel len klikol
	//takze sa zastavi timer
	timer->stop();
	//vyvola sa push event
	handlePush(*eaPush, *aaPush);

	//v pripade ze bol aj release tak sa vyvola aj release
	if (eaRel != NULL)
		handleRelease(*eaRel, *aaRel);
}


bool PickHandler::handleMove( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
	// Record mouse location for the button press
	// and move events.
	_mX = ea.getX();
	_mY = ea.getY();
	return false;
}

bool PickHandler::handleDoubleclick( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
	return false;
}

bool PickHandler::handleScroll( const osgGA::GUIEventAdapter& ea, GUIActionAdapter& aa  )
{
    if (selectionType == SelectionType::CLUSTER && !pickedClusters.empty()) {
        QLinkedList<osg::ref_ptr<Data::Cluster> >::const_iterator i = pickedClusters.constBegin();
        float scale = appConf->getValue("Viewer.Display.NodeDistanceScale").toFloat();
        while (i != pickedClusters.constEnd())
        {
            Layout::ShapeGetter_Cube * shapeGetter = (*i)->getShapeGetter();
            if (shapeGetter != NULL) {
                if (isCtrlPressed) {
//                    double size = 5;

//                    if( ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_UP) {
//                        size *= -1;
//                    }

//                    shapeGetter->changeSize(size);
//                    osg::Vec3f vector = shapeGetter->getSurfaceNodeX()->getTargetPosition() - shapeGetter->getCenterNode()->getTargetPosition();
//                    float length = vector.normalize();

//                    if (length > 10) {
//                        vector *= length / 5;
//                    }

//                    if( ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_UP) {
//                        vector *= -1;
//                    }
//                    osg::Vec3f newPosition = shapeGetter->getSurfaceNodeX()->getTargetPosition() + vector / scale;

//                    shapeGetter->getSurfaceNodeX()->setTargetPosition(newPosition);
//                    //shapeGetter->getSurfaceNodeX()->setCntPosition(newPosition);
                    //                    shapeGetter->getSurfaceNodeX()->getCurrentPosition(true);urre

                    double moveLength = 1;

                    if( ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_UP) {
                        moveLength *= -1;
                    }

                    if (isXPressed) {
                        shapeGetter->move(shapeGetter->getDistanceX() / 5 * moveLength,0,0);
                    } else if (isYPressed) {
                        shapeGetter->move(0,shapeGetter->getDistanceY() / 5 * moveLength,0);
                    } else if (isZPressed) {
                        shapeGetter->move(0,0,shapeGetter->getDistanceZ() / 5 * moveLength);
                    } else {
                        double avgDist = (shapeGetter->getDistanceX() + shapeGetter->getDistanceY() + shapeGetter->getDistanceZ()) / 3;
                        shapeGetter->move(avgDist / 5 * moveLength, avgDist / 5 * moveLength, avgDist / 5 * moveLength);
                    }

                }
            }
            ++i;
        }
        AppCore::Core::getInstance()->getLayoutThread()->wakeUp();
    }
    return false;
}

bool PickHandler::handleKeyUp( const osgGA::GUIEventAdapter& ea, GUIActionAdapter& aa  )
{
	if(ea.getKey() == osgGA::GUIEventAdapter::KEY_Control_R || ea.getKey() == osgGA::GUIEventAdapter::KEY_Control_L)
	{
		isCtrlPressed = false;
	}
	else if(ea.getKey() == osgGA::GUIEventAdapter::KEY_Shift_L || ea.getKey() == osgGA::GUIEventAdapter::KEY_Shift_R)
	{
		isShiftPressed = false;
	}
	else if(ea.getKey() == osgGA::GUIEventAdapter::KEY_Alt_L || ea.getKey() == osgGA::GUIEventAdapter::KEY_Alt_R)
	{
		isAltPressed = false;
	}
    else if(ea.getKey() == osgGA::GUIEventAdapter::KEY_X)
    {
        isXPressed = false;
    }
    else if(ea.getKey() == osgGA::GUIEventAdapter::KEY_Y)
    {
        isYPressed = false;
    }
    else if(ea.getKey() == osgGA::GUIEventAdapter::KEY_Z)
    {
        isZPressed = false;
    }
	return false;
}

bool PickHandler::handleKeyDown( const osgGA::GUIEventAdapter& ea, GUIActionAdapter& aa)
{
	if(ea.getKey() == osgGA::GUIEventAdapter::KEY_Control_R || ea.getKey() == osgGA::GUIEventAdapter::KEY_Control_L)
	{
		isCtrlPressed = true;
	}
	else if(ea.getKey() == osgGA::GUIEventAdapter::KEY_Shift_L || ea.getKey() == osgGA::GUIEventAdapter::KEY_Shift_R)
	{
		isShiftPressed = true;
	}
	else if(ea.getKey() == osgGA::GUIEventAdapter::KEY_Alt_L || ea.getKey() == osgGA::GUIEventAdapter::KEY_Alt_R)
	{
		isAltPressed = true;
	}
    else if(ea.getKey() == 'q')
    {
        Data::Graph * currentGraph = Manager::GraphManager::getInstance()->getActiveGraph();
        Util::ElementSelector::randomElementSelector(currentGraph->getNodes(), currentGraph->getEdges(), appConf->getValue("Viewer.PickHandler.AutopickedNodes").toInt(), this);
    }
    else if(ea.getKey() == 'w')
    {
        Data::Graph * currentGraph = Manager::GraphManager::getInstance()->getActiveGraph();
        Util::ElementSelector::weightedElementSelector(currentGraph->getNodes(), appConf->getValue("Viewer.PickHandler.AutopickedNodes").toInt(), this);
    }
    else if(ea.getKey() == osgGA::GUIEventAdapter::KEY_X)
    {
        isXPressed = true;
    }
    else if(ea.getKey() == osgGA::GUIEventAdapter::KEY_Y)
    {
        isYPressed = true;
    }
    else if(ea.getKey() == osgGA::GUIEventAdapter::KEY_Z)
    {
        isZPressed = true;
    }

	// FULLSCREEN
	else if(ea.getKey() == 'l' || ea.getKey() == 'L')
	{
		bool hideToolbars = appConf->getValue("Viewer.Fullscreen").toInt();

		if( AppCore::Core::getInstance()->getCoreWindow()->isFullScreen()){
			AppCore::Core::getInstance()->getCoreWindow()->menuBar()->show();
			AppCore::Core::getInstance()->getCoreWindow()->statusBar()->show();
			AppCore::Core::getInstance()->getCoreWindow()->showNormal();

			if(hideToolbars){
				QList<QToolBar *> toolbars = AppCore::Core::getInstance()->getCoreWindow()->findChildren<QToolBar *>();
				QListIterator<QToolBar *> i(toolbars);
				while (i.hasNext()){
					i.next()->show();
				}
			}
		} else {
			AppCore::Core::getInstance()->getCoreWindow()->menuBar()->hide();
			AppCore::Core::getInstance()->getCoreWindow()->statusBar()->hide();
			AppCore::Core::getInstance()->getCoreWindow()->showFullScreen();

			if(hideToolbars){
				QList<QToolBar *> toolbars = AppCore::Core::getInstance()->getCoreWindow()->findChildren<QToolBar *>();
				QListIterator<QToolBar *> i(toolbars);
				while (i.hasNext()){
					i.next()->hide();
				}
			}

		}
	}

	// toolbars only
	else if(ea.getKey() == 't' || ea.getKey() == 'T')
	{
		static bool toolbars = true;
		if( toolbars ){
			toolbars = !toolbars;
			QList<QToolBar *> toolbars = AppCore::Core::getInstance()->getCoreWindow()->findChildren<QToolBar *>();
			QListIterator<QToolBar *> i(toolbars);
			while (i.hasNext()){
				i.next()->show();
			}

		} else {
			toolbars = !toolbars;
			QList<QToolBar *> toolbars = AppCore::Core::getInstance()->getCoreWindow()->findChildren<QToolBar *>();
			QListIterator<QToolBar *> i(toolbars);
			while (i.hasNext()){
				i.next()->hide();
			}
		}

	}


	return false;
}

bool PickHandler::handleRelease( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
	osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>( &aa );

	if (!viewer)
		return false;

	// If the mouse hasn't moved since the last
	// button press or move event, perform a
	// pick. (Otherwise, the trackball
	// manipulator will handle it.)

	leftButtonPressed = false;

	if (pickMode == PickMode::MULTI && isDrawingSelectionQuad)
	{
		float x, y, w, h;

		if (origin_mX_normalized < ea.getXnormalized())
		{
			x = origin_mX_normalized;
			w = ea.getXnormalized();
		}
		else
		{
			x = ea.getXnormalized();
			w = origin_mX_normalized ;
		}

		if (origin_mY_normalized < ea.getYnormalized())
		{
			y = origin_mY_normalized;
			h = ea.getYnormalized();
		}
		else
		{
			y = ea.getYnormalized();
			h = origin_mY_normalized;
		}

		pick( x, y, w, h, viewer );

		if (coreGraph->getCustomNodeList()->contains(group))
		{
			coreGraph->getCustomNodeList()->removeOne(group);
		}

		isDrawingSelectionQuad = false;

		return true;
	}

	if(isManipulatingNodes)
	{
		isManipulatingNodes = false;
		setSelectedNodesInterpolation(true);
	}

	return false;
}

bool PickHandler::handleDrag( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
	_mX =  ea.getX();
	_mY =  ea.getY();

	if (pickMode == PickMode::MULTI && isDrawingSelectionQuad)
	{
		osg::ref_ptr<osg::Vec3Array> coordinates = new osg::Vec3Array;

		coordinates->push_back(osg::Vec3(origin_mX, origin_mY, -1));
		coordinates->push_back(osg::Vec3(origin_mX,		  _mY, -1));
		coordinates->push_back(osg::Vec3(	   _mX,       _mY, -1));
		coordinates->push_back(osg::Vec3(	   _mX, origin_mY, -1));

		selectionQuad->getDrawable(0)->asGeometry()->setVertexArray(coordinates);
	}
    else if (selectionType == SelectionType::CLUSTER && pickMode == PickMode::NONE && leftButtonPressed) {
        osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>( &aa );
        if (!viewer)
            return false;
        return dragCluster(viewer);
    }
	else if (pickMode == PickMode::NONE && leftButtonPressed)
	{
		Network::Client * client = Network::Client::getInstance();
		if (client->isConnected()){
			client -> setNodesExcludedFromUpdate(pickedNodes);
		} else {
			Network::Server * server = Network::Server::getInstance();
			server -> setSelectedNodes(pickedNodes);
		}

		if(!isManipulatingNodes)
		{
			isManipulatingNodes = true;
			setSelectedNodesInterpolation(false);
			toggleSelectedNodesFixedState(true);
		}

		osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>( &aa );

		if (!viewer)
			return false;

		return dragNode(viewer);
	}

	return false;
}

bool PickHandler::handlePush( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
	if (ea.getButtonMask() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
	{
		leftButtonPressed = true;

		origin_mX = ea.getX();
		origin_mY = ea.getY();

		origin_mX_normalized = ea.getXnormalized();
		origin_mY_normalized = ea.getYnormalized();

		if (pickMode != PickMode::NONE && !isShiftPressed && !isCtrlPressed)
		{
            unselectPickedClusters();
            unselectPickedNodes();
			unselectPickedEdges();
		}

		osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>( &aa );

		if (!viewer)
			return false;

		if (pickMode == PickMode::MULTI)
		{
			isDrawingSelectionQuad = true;

			drawSelectionQuad(origin_mX, origin_mY, viewer);
		}

		else
		{
			return pick(ea.getXnormalized() - 0.00005f, ea.getYnormalized() - 0.00005f, ea.getXnormalized() + 0.00005f, ea.getYnormalized() + 0.00005f, viewer );

		}
	}

	_mX = ea.getX();
	_mY = ea.getY();
	return false;
}


bool PickHandler::pick( const double xMin, const double yMin, const double xMax, const double yMax, osgViewer::Viewer* viewer ){

	if (!viewer->getSceneData())
		// Nothing to pick.
		return false;

	osgUtil::PolytopeIntersector* picker = new osgUtil::PolytopeIntersector(osgUtil::Intersector::PROJECTION, xMin, yMin, xMax, yMax);
	osgUtil::IntersectionVisitor iv( picker );

	viewer->getCamera()->accept( iv );

	bool result = false;

	if (picker->containsIntersections())
	{
		osgUtil::PolytopeIntersector::Intersections intersections = picker->getIntersections();

		for(osgUtil::PolytopeIntersector::Intersections::iterator hitr = intersections.begin(); hitr != intersections.end(); hitr++)
		{
			if (!hitr->nodePath.empty())
			{
                osg::NodePath nodePath = hitr->nodePath;

				if (nodePath.size() > 1)
				{
					if (!isDrawingSelectionQuad)
					{
						result = doSinglePick(nodePath, hitr->primitiveIndex);
						break;
					}
					else
					{
						bool nodePicked = false;
						bool edgePicked = false;


						if (selectionType == SelectionType::NODE || selectionType == SelectionType::ALL)
							nodePicked = doNodePick(nodePath);
						if (!nodePicked){
							if (selectionType == SelectionType::EDGE || selectionType == SelectionType::ALL)
								edgePicked = doEdgePick(nodePath, hitr->primitiveIndex);
						}


						result = result || nodePicked || edgePicked ;
					}
				}
			}
		}
	}

	return result;
}

bool PickHandler::doSinglePick(osg::NodePath nodePath, unsigned int primitiveIndex)
{
    if (selectionType == SelectionType::NODE)
		return doNodePick(nodePath);
	else if (selectionType == SelectionType::EDGE)
		return doEdgePick(nodePath, primitiveIndex);
    else if (selectionType == SelectionType::CLUSTER)
        return doClusterPick(nodePath);
    else
		return (doNodePick(nodePath) || doEdgePick(nodePath, primitiveIndex));
}

bool PickHandler::doNodePick(osg::NodePath nodePath)
{
	Data::Node * n = dynamic_cast<Data::Node *>(nodePath[nodePath.size() - 1]);

	if (n != NULL)
	{
		if (isAltPressed && pickMode == PickMode::NONE && !isShiftPressed)
		{
			cameraManipulator->setCenter(n->targetPosition());
		}
		else if (isAltPressed && pickMode == PickMode::NONE && isShiftPressed)
		{
			if (appConf->getValue("Viewer.PickHandler.SelectInterestPoints").toInt() == 1)
			{
				Data::Graph * currentGraph = Manager::GraphManager::getInstance()->getActiveGraph();
				Util::ElementSelector::weightedElementSelector(currentGraph->getNodes(), appConf->getValue("Viewer.PickHandler.AutopickedNodes").toInt(), this);
			}

			bool wasEmpty = false;
			if (pickedNodes.isEmpty())
			{
				pickedNodes.append(n);
				wasEmpty = true;
			}

			if (appConf->getValue("Viewer.Display.CameraPositions").toInt() == 1)
			{
				n->setColor(osg::Vec4(0, 1, 0, 1));
			}

			cameraManipulator->setNewPosition(n->getCurrentPosition(), getSelectionCenter(false), getSelectedNodes()->toStdList(), getSelectedEdges()->toStdList());

			if (wasEmpty)
				pickedNodes.removeFirst();
		}
		else if (pickMode != PickMode::NONE)
		{
			if (!pickedNodes.contains(n))
			{
				pickedNodes.append(n);
				n->setSelected(true);
			}

			if (isCtrlPressed)
				unselectPickedNodes(n);

			return true;
		}
	}

	return false;
}

bool PickHandler::doEdgePick(osg::NodePath nodePath, unsigned int primitiveIndex)
{
	osg::Geode * geode = dynamic_cast<osg::Geode *>(nodePath[nodePath.size() - 1]);

	if (geode != 0)
	{
		osg::Drawable * d = geode->getDrawable(0);
		osg::Geometry * geometry = d->asGeometry();

		if (geometry != NULL)
		{
			// zmena (plesko): ak vyber zachytil avatara, nastal segmentation fault,
			// lebo sa vyberal neexistujuci primitiveSet
			Data::Edge * e;
			if (geometry->getNumPrimitiveSets() > primitiveIndex) {
				e = dynamic_cast<Data::Edge *>(geometry->getPrimitiveSet(primitiveIndex));
			} else {
				return false;
			}
			// koniec zmeny

			if (e != NULL)
			{
				if (isAltPressed && pickMode == PickMode::NONE && !isShiftPressed)
				{
					osg::ref_ptr<osg::Vec3Array> coords = e->getCooridnates();

					cameraManipulator->setCenter(DataHelper::getMassCenter(coords));
					cameraManipulator->setDistance(Util::ApplicationConfig::get()->getValue("Viewer.PickHandler.PickedEdgeDistance").toFloat());
				}
				else if (isAltPressed && pickMode == PickMode::NONE && isShiftPressed)
				{
					if (appConf->getValue("Viewer.PickHandler.SelectInterestPoints").toInt() == 1)
					{
						Data::Graph * currentGraph = Manager::GraphManager::getInstance()->getActiveGraph();
						Util::ElementSelector::weightedElementSelector(currentGraph->getNodes(), appConf->getValue("Viewer.PickHandler.AutopickedNodes").toInt(), this);
					}

					bool wasEmpty = false;
					if (pickedEdges.isEmpty())
					{
						pickedEdges.append(e);
						wasEmpty = true;
					}

					osg::Vec3f edgeCenter = (e->getSrcNode()->getCurrentPosition() + e->getDstNode()->getCurrentPosition()) / 2;

					cameraManipulator->setNewPosition(edgeCenter, getSelectionCenter(false), getSelectedNodes()->toStdList(), getSelectedEdges()->toStdList());

					if (wasEmpty)
						pickedEdges.removeFirst();
				}
				else if (pickMode != PickMode::NONE)
				{
					if (!pickedEdges.contains(e))
					{
						pickedEdges.append(e);
						e->setSelected(true);
					}

					if (isCtrlPressed)
						unselectPickedEdges(e);

					return true;
				}

				return true;
			}
		}
	}

	return false;
}

bool PickHandler::doClusterPick(osg::NodePath nodePath)
{
    std::string clusterIdStr;
    nodePath[nodePath.size() - 1]->getUserValue("id", clusterIdStr);
    qlonglong clusterId = QString::fromStdString(clusterIdStr).toLongLong();

    osg::ref_ptr<Data::Cluster> cluster = Clustering::Clusterer::getInstance().getClusters().value(clusterId);

    if (cluster != NULL)
    {
        if (pickMode == PickMode::SINGLE)
        {
            if (!pickedNodes.contains(cluster->getRandomNode())) {
                cluster->setSelected(true, &pickedNodes);
                pickedClusters.append(cluster);
                AppCore::Core::getInstance()->getCoreWindow()->setRepulsiveForceInsideCluster(cluster->getRepulsiveForceInside());
            }

            if (isCtrlPressed) {
                cluster->setSelected(false, &pickedNodes);
                pickedClusters.clear();
                AppCore::Core::getInstance()->getCoreWindow()->hideRepulsiveForceSpinBox();
            }
            return true;
        }
    }

    return false;

}

bool PickHandler::dragNode(osgViewer::Viewer * viewer)
{
	QLinkedList<osg::ref_ptr<Data::Node> >::const_iterator i = pickedNodes.constBegin();

	osg::Matrixd&  viewM = viewer->getCamera()->getViewMatrix();
	osg::Matrixd&  projM = viewer->getCamera()->getProjectionMatrix();
	osg::Matrixd screenM = viewer->getCamera()->getViewport()->computeWindowMatrix();

	osg::Matrixd compositeM = viewM*projM*screenM;
	osg::Matrixd compositeMi = compositeMi.inverse(compositeM);

	float scale = appConf->getValue("Viewer.Display.NodeDistanceScale").toFloat();

	while (i != pickedNodes.constEnd())
	{
		osg::Vec3f screenPoint = (*i)->targetPositionConstRef() * compositeM;
		osg::Vec3f newPosition = osg::Vec3f(screenPoint.x() - (origin_mX - _mX) / scale, screenPoint.y() - (origin_mY - _mY) / scale, screenPoint.z());

		(*i)->setTargetPosition(newPosition * compositeMi);
		++i;
	}

	origin_mX = _mX;
	origin_mY = _mY;

	AppCore::Core::getInstance()->getLayoutThread()->wakeUp();

	return (pickedNodes.size() > 0);
}

bool PickHandler::dragCluster(osgViewer::Viewer * viewer)
{
    QLinkedList<osg::ref_ptr<Data::Cluster> >::const_iterator i = pickedClusters.constBegin();
    QLinkedList<osg::ref_ptr<Data::Node> >::const_iterator n = pickedNodes.constBegin();

    osg::Matrixd&  viewM = viewer->getCamera()->getViewMatrix();
    osg::Matrixd&  projM = viewer->getCamera()->getProjectionMatrix();
    osg::Matrixd screenM = viewer->getCamera()->getViewport()->computeWindowMatrix();

    osg::Matrixd compositeM = viewM*projM*screenM;
    osg::Matrixd compositeMi = compositeMi.inverse(compositeM);

    float scale = appConf->getValue("Viewer.Display.NodeDistanceScale").toFloat();

    while (i != pickedClusters.constEnd())
    {
        Layout::ShapeGetter_Cube * shape = (*i)->getShapeGetter();

        if (shape != NULL) {
            osg::Vec3d screenPoint = shape->getCenterNode()->getTargetPosition() * compositeM;
            osg::Vec3d newPosition = osg::Vec3d(screenPoint.x() - (origin_mX - _mX) / scale, screenPoint.y() - (origin_mY - _mY) / scale, screenPoint.z());
            shape->getCenterNode()->setTargetPosition(newPosition * compositeMi);
            //shape->getCenterNode()->setCurrentPosition(newPosition * compositeMi);
            shape->getCenterNode()->getCurrentPosition(true);

            screenPoint = shape->getSurfaceNodeX()->getTargetPosition() * compositeM;
            newPosition = osg::Vec3d(screenPoint.x() - (origin_mX - _mX) / scale, screenPoint.y() - (origin_mY - _mY) / scale, screenPoint.z());
            shape->getSurfaceNodeX()->setTargetPosition(newPosition * compositeMi);
            shape->getSurfaceNodeX()->getCurrentPosition(true);

            screenPoint = shape->getSurfaceNodeY()->getTargetPosition() * compositeM;
            newPosition = osg::Vec3d(screenPoint.x() - (origin_mX - _mX) / scale, screenPoint.y() - (origin_mY - _mY) / scale, screenPoint.z());
            shape->getSurfaceNodeY()->setTargetPosition(newPosition * compositeMi);
            shape->getSurfaceNodeY()->getCurrentPosition(true);

            screenPoint = shape->getSurfaceNodeZ()->getTargetPosition() * compositeM;
            newPosition = osg::Vec3d(screenPoint.x() - (origin_mX - _mX) / scale, screenPoint.y() - (origin_mY - _mY) / scale, screenPoint.z());
            shape->getSurfaceNodeZ()->setTargetPosition(newPosition * compositeMi);
            shape->getSurfaceNodeZ()->getCurrentPosition(true);
        }
        ++i;
    }
//    while (n != pickedNodes.constEnd())
//    {
//        osg::Vec3f screenPoint = (*n)->targetPositionConstRef() * compositeM;
//        osg::Vec3f newPosition = osg::Vec3f(screenPoint.x() - (origin_mX - _mX) / scale, screenPoint.y() - (origin_mY - _mY) / scale, screenPoint.z());

//        (*n)->setTargetPosition(newPosition * compositeMi);
//        (*n)->getCurrentPosition(true);
//        ++n;
//    }

    origin_mX = _mX;
    origin_mY = _mY;

    AppCore::Core::getInstance()->getLayoutThread()->wakeUp();

    return (pickedClusters.size() > 0);
}

void PickHandler::drawSelectionQuad(float origin_mX, float origin_mY, osgViewer::Viewer * viewer)
{
	osg::ref_ptr<osg::StateSet> quadStateSet = new osg::StateSet;
	quadStateSet->setMode(GL_BLEND,osg::StateAttribute::ON);
	quadStateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
	quadStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	quadStateSet->setAttributeAndModes(new osg::BlendFunc, osg::StateAttribute::ON);
	quadStateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	quadStateSet->setRenderBinDetails( 11, "RenderBin");

	osg::ref_ptr<osg::Vec3Array> coordinates = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;

	coordinates->push_back(osg::Vec3(origin_mX, origin_mY, -1));
	coordinates->push_back(osg::Vec3(origin_mX, origin_mY, -1));
	coordinates->push_back(osg::Vec3(origin_mX, origin_mY, -1));
	coordinates->push_back(osg::Vec3(origin_mX, origin_mY, -1));

	colors->push_back(osg::Vec4(0.2f, 0.2f, 1.0f, 0.3f));  // color of selection restangle

	geometry->setVertexArray(coordinates);
	geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
	geometry->setColorArray(colors);
	geometry->setStateSet(quadStateSet);
#ifdef BIND_PER_PRIMITIVE
	geometry->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE);
#else
	geometry->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);
#endif
	selectionQuad = new osg::Geode;
	selectionQuad->addDrawable(geometry);

	osgViewer::ViewerBase::Windows win;
	viewer->getWindows(win);

	int x, y, w, h;
	win.at(0)->getWindowRectangle(x, y, w, h);

	osg::ref_ptr<osg::Projection> projectionMatrix = new osg::Projection;
	projectionMatrix->setMatrix(osg::Matrix::ortho2D(x, w, y, h));

	osg::ref_ptr<osg::MatrixTransform> modelViewMatrix = new osg::MatrixTransform;
	modelViewMatrix->setMatrix(osg::Matrix::identity());

	modelViewMatrix->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

	group = new osg::Group;

	group->addChild(projectionMatrix);
	projectionMatrix->addChild(modelViewMatrix);
	modelViewMatrix->addChild(selectionQuad);

	coreGraph->getCustomNodeList()->push_back(group);
}

void PickHandler::toggleSelectedNodesFixedState(bool isFixed)
{
	QLinkedList<osg::ref_ptr<Data::Node> >::const_iterator i = pickedNodes.constBegin();

	Network::Client * client = Network::Client::getInstance();
	Network::Server * server = Network::Server::getInstance();

	while (i != pickedNodes.constEnd())
	{
		if (client->isConnected()) {
			client->sendFixNodeState((*i)->getId(), isFixed);
		} else {
			(*i)->setFixed(isFixed);  // fixed nodes cannot by draged, layouter don compute new position for them
			server->sendFixNodeState((*i)->getId(), isFixed);
		}
		++i;
	}
}

void PickHandler::unselectPickedClusters() {
    QLinkedList<osg::ref_ptr<Data::Cluster> >::const_iterator i = pickedClusters.constBegin();

    while (i != pickedClusters.constEnd())
    {
        (*i)->setSelected(false, &pickedNodes);
        ++i;
    }
    AppCore::Core::getInstance()->getCoreWindow()->hideRepulsiveForceSpinBox();
    pickedClusters.clear();
}

void PickHandler::unselectPickedNodes(osg::ref_ptr<Data::Node> node)
{
	if (node == NULL)
	{
		QLinkedList<osg::ref_ptr<Data::Node> >::const_iterator i = pickedNodes.constBegin();

		while (i != pickedNodes.constEnd())
		{
			(*i)->setSelected(false);
			++i;
		}

		pickedNodes.clear();
	}
	else
	{
		node->setSelected(false);
		pickedNodes.removeOne(node);
	}
}

void PickHandler::unselectPickedEdges(osg::ref_ptr<Data::Edge> edge)
{
	if (edge == NULL)
	{
		QLinkedList<osg::ref_ptr<Data::Edge> >::const_iterator i = pickedEdges.constBegin();

		while (i != pickedEdges.constEnd())
		{
			(*i)->setSelected(false);
			++i;
		}

		pickedEdges.clear();
	}
	else
	{
		edge->setSelected(false);
		pickedEdges.removeOne(edge);
	}
}

osg::Vec3 PickHandler::getSelectionCenter(bool nodesOnly)
{
	osg::ref_ptr<osg::Vec3Array> coordinates = new osg::Vec3Array;
	float scale = appConf->getValue("Viewer.Display.NodeDistanceScale").toFloat();

	if (!nodesOnly)
	{
		QLinkedList<osg::ref_ptr<Data::Edge> >::const_iterator ei = pickedEdges.constBegin();

		while (ei != pickedEdges.constEnd())
		{
			coordinates->push_back(DataHelper::getMassCenter((*ei)->getCooridnates()));
			++ei;
		}
	}

	QLinkedList<osg::ref_ptr<Data::Node> >::const_iterator ni = pickedNodes.constBegin();

	while (ni != pickedNodes.constEnd())
	{
		// MERGE BEGIN
		// sivak
		coordinates->push_back((*ni)->targetPositionConstRef());
		// plesko-zeler
		//coordinates->push_back((*ni)->getCurrentPosition());
		// povodne
		//coordinates->push_back((*ni)->getTargetPosition());
		// MERGE END
		++ni;
	}

	osg::Vec3 center;

	if (coordinates->size() > 0)
		center = Vwr::DataHelper::getMassCenter(coordinates);

	return center * scale;
}

void PickHandler::setSelectedNodesInterpolation(bool state)
{
	QLinkedList<osg::ref_ptr<Data::Node> >::const_iterator i = pickedNodes.constBegin();

	while (i != pickedNodes.constEnd())
	{
		(*i)->setUsingInterpolation(state);
		++i;
	}
}

osg::ref_ptr<Data::Node> PickHandler::getPickedNodeWithMaxEdgeCount(){
	int maxEdges=0;
	osg::ref_ptr<Data::Node> rootNode;
	QLinkedList<osg::ref_ptr<Data::Node> >::const_iterator itNode;
	for ( itNode = pickedNodes.constBegin (); itNode != pickedNodes.constEnd (); itNode++) {
		int actEdges = (*itNode)->getEdges()->size();
		if ( actEdges>maxEdges){
			rootNode=(*itNode);
			maxEdges=actEdges;
		}

	}
	return rootNode;
}

#pragma GCC diagnostic pop
