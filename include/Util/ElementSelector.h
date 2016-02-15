#ifndef UTIL_ELEMENT_SELECTOR
#define UTIL_ELEMENT_SELECTOR 1

#include <QMap>
#include <osg/ref_ptr>


namespace Vwr {
class PickHandler;
}

namespace Data {
class Node;
class Edge;
}

namespace Util {
class ElementSelector
{
public:
    static void randomElementSelector( QMap<QString, osg::ref_ptr<Data::Node> >* nodes, QMap<QString, osg::ref_ptr<Data::Edge> >* edges, int num, Vwr::PickHandler* pickhandler );
    static void weightedElementSelector( QMap<QString, osg::ref_ptr<Data::Node> >* nodes, int num, Vwr::PickHandler* pickhandler );
};
}

#endif
