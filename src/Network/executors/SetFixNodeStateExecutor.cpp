#include "Network/executors/SetFixNodeStateExecutor.h"
#include "Manager/Manager.h"
#include "Network/Client.h"
#include "Network/Server.h"
#include "Data/Node.h"

namespace Network {

void SetFixNodeStateExecutor::execute_client()
{
	int id;
	bool state;

	*stream >> id >> state;


	Data::Graph* currentGraph = Manager::GraphManager::getInstance()->getActiveGraph();
    QMap<QString, osg::ref_ptr<Data::Node> >* nodes = currentGraph -> getNodes();
	Client* client = Client::getInstance();
    if ( nodes->contains( QString::number( id ) ) ) {
        Data::Node* node = *nodes->find( QString::number( id ) );
		this->SetFixNodeState( node, state );
		if ( client->selected_nodes.contains( node ) ) {
			client->selected_nodes.removeOne( node );
		}
	}
	else {
        QMap<QString, osg::ref_ptr<Data::Node> >* mergeNodes = currentGraph->getMetaNodes();
        if ( mergeNodes->contains( QString::number( id ) ) ) {
            Data::Node* node = *mergeNodes->find( QString::number( id ) );
			this->SetFixNodeState( node,state );
			if ( client->selected_nodes.contains( node ) ) {
				client->selected_nodes.removeOne( node );
			}
		}
	}

}

void SetFixNodeStateExecutor::execute_server()
{
	int id;
	bool state;

	*stream >> id >> state;

	Data::Graph* currentGraph = Manager::GraphManager::getInstance()->getActiveGraph();
    QMap<QString, osg::ref_ptr<Data::Node> >* nodes = currentGraph -> getNodes();
    if ( nodes->contains( QString::number( id ) ) ) {
        this->SetFixNodeState( *nodes->find( QString::number( id ) ), state );
	}
	else {
        QMap<QString, osg::ref_ptr<Data::Node> >* mergeNodes = currentGraph->getMetaNodes();
        if ( mergeNodes->contains( QString::number( id ) ) ) {
            this->SetFixNodeState( *mergeNodes->find( QString::number( id ) ),state );
		}
	}

	Server* server = Server::getInstance();
	server->sendFixNodeState( id, state );

	if ( ( ( QOSG::CoreWindow* )server->getCoreWindowReference() )->playing() ) {
		server->getLayoutThread()->play();
	}

}

void SetFixNodeStateExecutor::SetFixNodeState( Data::Node* node, bool state )
{

	node->setFixed( state );
	node->setUsingInterpolation( !state );

}

} // namespace Network
