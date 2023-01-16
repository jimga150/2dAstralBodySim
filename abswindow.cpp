#include "abswindow.h"

ABSWindow::ABSWindow(){

    QScreen* screen = this->screen();
    float fps = screen->refreshRate();

    float targetFPS = 60;
    frame_divisor = qMax((int)floor(fps/targetFPS), 1);
    fps /= frame_divisor;

    float framerate_s_f = 1.0/fps;
    this->timeStep_s = framerate_s_f;

    printf("Refresh rate (adjusted to between 60 and 119.999 FPS: %f FPS (%f ms)\n", fps, framerate_s_f*MILLIS_PER_SECOND);

    this->setupWindow();

    this->world = new b2World(b2Vec2(0, 0)); //no global gravity

    this->bodydef_template.type = b2_dynamicBody;

    this->circle_shape.m_radius = 10;

    fixturedef_template.density = 1;
    fixturedef_template.shape = &circle_shape;

    this->bodydef_template.position.Set(50, 0);
    b2Body* b = this->world->CreateBody(&this->bodydef_template);
    b->CreateFixture(&fixturedef_template);

    this->bodydef_template.position.Set(-50, 0);
    b = this->world->CreateBody(&this->bodydef_template);
    b->CreateFixture(&fixturedef_template);
}

ABSWindow::~ABSWindow(){
    delete this->world;
}

void ABSWindow::resizeEvent(QResizeEvent* event){
    this->window_size = event->size();
}

void ABSWindow::mousePressEvent(QMouseEvent *ev){

}

void ABSWindow::mouseReleaseEvent(QMouseEvent *ev){
    b2Vec2 pos = this->scrnPtToPhysPt(ev->position());
    this->bodydef_template.position = pos;
    b2Body* b = this->world->CreateBody(&this->bodydef_template);
    b->CreateFixture(&fixturedef_template);
    printf("creating body at (%f, %f)\n", pos.x, pos.y);
}

void ABSWindow::mouseMoveEvent(QMouseEvent *ev){

}

void ABSWindow::setupWindow(){
    QScreen* screen = this->screen();

    QRect screenRect = screen->availableGeometry();
    int screen_width = screenRect.width();
    int screen_height = screenRect.height();

    this->window_size = QSize(screen_width, screen_height);

    this->setGeometry(QRect(QPoint(0, 0), this->window_size));
}

void ABSWindow::render(QPainter &painter){

    painter.setPen(Qt::SolidLine);
    painter.setPen(QColor(255, 0, 0));
    painter.setBrush(Qt::NoBrush);

    for (b2Body* b = this->world->GetBodyList(); b; b = b->GetNext()){
        this->drawBodyTo(&painter, b);
    }

//    printf("render\n");

}

void ABSWindow::drawBodyTo(QPainter* painter, b2Body* body){

    RETURN_IF_NULL(body);
    RETURN_IF_NULL(painter);

    painter->save();
    b2Vec2 pos = body->GetPosition();
    QPointF body_center_px = this->physPtToScrnPt(pos);
//    printf("Body position: (%f, %f) m\n", pos.x, pos.y);
    painter->translate(body_center_px.x(), body_center_px.y());

//    //https://stackoverflow.com/questions/8881923/how-to-convert-a-pointer-value-to-qstring
//    QString ptrStr = QString("0x%1").arg(reinterpret_cast<quintptr>(body),QT_POINTER_SIZE * 2, 16, QChar('0'));
//    //QString coordstr = QString("(%1, %2)").arg(body->GetPosition().x).arg(body->GetPosition().y);
//    //printf("\t%s: %s\n", ptrStr.toUtf8().constData(), coordstr.toUtf8().constData());

//    painter->drawText(QPoint(0, 0), ptrStr);

    for (b2Fixture* f = body->GetFixtureList(); f; f = f->GetNext()){
        switch(f->GetType()){
        case b2Shape::e_polygon:{
            b2PolygonShape shape = *static_cast<b2PolygonShape*>(f->GetShape());
            int numpoints = shape.m_count;
            std::vector<QPointF> points;
            for (int i = 0; i < numpoints; i++){
                points.push_back(
                            QPointF(
                                shape.m_vertices[i].x*this->viewscale_p_m,
                                shape.m_vertices[i].y*this->viewscale_p_m
                                )
                            );
                //printf("Point: (%f, %f)\n", points[i].x(), points[i].y());
            }
            painter->drawPolygon(&points[0], numpoints);

        }
            break;
        case b2Shape::e_circle:{
            b2CircleShape shape = *static_cast<b2CircleShape*>(f->GetShape());
            QPointF center(
                        shape.m_p.x*this->viewscale_p_m,
                        shape.m_p.y*this->viewscale_p_m
                        );
            double radius = static_cast<double>(shape.m_radius);
            radius *= this->viewscale_p_m;
            painter->drawEllipse(center, radius, radius);
        }
            break;
        case b2Shape::e_edge:{
            b2EdgeShape shape = *static_cast<b2EdgeShape*>(f->GetShape());
            QPointF p1 = QPointF(
                        shape.m_vertex1.x*this->viewscale_p_m,
                        shape.m_vertex1.y*this->viewscale_p_m
                        );
            QPointF p2 = QPointF(
                        shape.m_vertex2.x*this->viewscale_p_m,
                        shape.m_vertex2.y*this->viewscale_p_m
                        );
            painter->drawLine(p1, p2);
        }
            break;
        case b2Shape::e_chain:{
            b2ChainShape shape = *static_cast<b2ChainShape*>(f->GetShape());
            QPainterPath path;
            path.moveTo(
                        shape.m_vertices[0].x,
                    shape.m_vertices[0].y //I HATE this indentation. Why, Qt?
                    );
            for (int i = 1; i < shape.m_count; i++){
                path.lineTo(
                            shape.m_vertices[i].x*this->viewscale_p_m,
                            shape.m_vertices[i].y*this->viewscale_p_m
                            );
            }
            painter->drawPath(path);
        }
            break;
        default:
            fprintf(stderr, "Fixture in body has undefined shape type!\n");
            return;
        }
    }

//    b2Vec2 com_m = body->GetLocalCenter();
//    painter->drawEllipse(QPointF(com_m.x, com_m.y)*this->viewscale_p_m, 10, 5);

    painter->restore();
}

QPointF ABSWindow::physPtToScrnPt(b2Vec2 worldPoint_m){
    return QPointF(
                (worldPoint_m.x - this->viewcenter_m.x)*this->viewscale_p_m + this->window_size.width()/2,
                (worldPoint_m.y - this->viewcenter_m.y)*this->viewscale_p_m + this->window_size.height()/2
                );
}

b2Vec2 ABSWindow::scrnPtToPhysPt(QPointF screenPoint_p){
    return b2Vec2(
                (screenPoint_p.x() - this->window_size.width()/2)/this->viewscale_p_m + this->viewcenter_m.x,
                (screenPoint_p.y() - this->window_size.height()/2)/this->viewscale_p_m + this->viewcenter_m.y
                );
}

void ABSWindow::doGameStep(){

    for (b2Body* b = this->world->GetBodyList(); b; b = b->GetNext()){
        b2Vec2 ForceAccum = b2Vec2(0, 0);
        for (b2Body* ob = this->world->GetBodyList(); ob; ob = ob->GetNext()){
            if (ob == b) continue;
            b2Vec2 vect_to_ob = ob->GetWorldCenter() - b->GetWorldCenter();
            float force_mag = big_G*ob->GetMass()*b->GetMass()/vect_to_ob.LengthSquared();
//            printf("Force magnitude between b and ob: %f N\n", force_mag);
            b2Vec2 force_to_add = vect_to_ob;
            force_to_add.Normalize();
            force_to_add *= force_mag;
//            printf("Adding force: x = %f, y = %f\n", force_to_add.x, force_to_add.y);
            ForceAccum += force_to_add;
        }
        b->ApplyLinearImpulseToCenter(ForceAccum, true);
    }

    int velocityIterations = 8;
    int positionIterations = 3;
    this->world->Step(this->timeStep_s, velocityIterations, positionIterations);

//    printf("step: ts = %f s\n", this->timeStep_s);
}
