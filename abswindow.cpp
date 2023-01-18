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

    this->contactlistener = new ABSContactListener;
    this->world->SetContactListener(this->contactlistener);

    this->bodydef_template.type = b2_dynamicBody;

    this->circle_shape.m_radius = 10;

    fixturedef_template.density = 1;
    fixturedef_template.shape = &circle_shape;

}

ABSWindow::~ABSWindow(){
    delete this->world;
}

void ABSWindow::resizeEvent(QResizeEvent* event){
    this->window_size = event->size();
}

void ABSWindow::mousePressEvent(QMouseEvent *ev){

    if (this->mouse_down_create_body || this->mouse_down_drag_view) return;

    switch(ev->button()){
    case Qt::LeftButton:
        this->mouse_down_create_body = true;
        break;
    case Qt::RightButton:
        this->mouse_down_drag_view = true;
        break;
    default:
        //do nothing
        break;
    }

    this->start_click_pos = ev->position();
    this->end_click_pos = ev->position();
}

void ABSWindow::mouseReleaseEvent(QMouseEvent *ev){

    this->end_click_pos = ev->position();

    if (this->mouse_down_create_body){

        QPointF velocity_p = this->end_click_pos - this->start_click_pos;
        b2Vec2 velocity_m(velocity_p.x()/this->viewscale_p_m, velocity_p.y()/this->viewscale_p_m);

        b2Vec2 pos = this->scrnPtToPhysPt(this->start_click_pos);

        this->createBody(this->default_body_radius_m, pos, velocity_m);

        //printf("creating body at (%f, %f)\n", pos.x, pos.y);

        this->mouse_down_create_body = false;

    } else if (this->mouse_down_drag_view){
        this->mouse_down_drag_view = false;
    }

}

void ABSWindow::mouseMoveEvent(QMouseEvent *ev){

    if (!this->mouse_down_create_body && !this->mouse_down_drag_view) return;

    QPointF last_drag_pos = this->end_click_pos;

    this->end_click_pos = ev->position();

    if (this->mouse_down_drag_view){
        QPointF view_offset_p = last_drag_pos - this->end_click_pos;
        b2Vec2 view_offset_m(view_offset_p.x()/this->viewscale_p_m, view_offset_p.y()/this->viewscale_p_m);

        this->viewcenter_m += view_offset_m;
    }
}

void ABSWindow::wheelEvent(QWheelEvent *ev){
    this->viewscale_p_m *= 1 + ev->pixelDelta().y()*0.001;
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

    if (this->enable_gravfield && this->world->GetBodyCount() > 0){

        painter.setPen(Qt::SolidLine);
        painter.setPen(QColor(150, 150, 150));
        painter.setBrush(Qt::NoBrush);

        int num_rows = this->gravfield_rowscols;
        int num_cols = this->gravfield_rowscols;

        int row_interval = this->window_size.height()/num_rows;
        int col_interval = this->window_size.width()/num_cols;

        int min_interval = row_interval;
        if (col_interval < row_interval) min_interval = col_interval;

        float phys_min_interval = min_interval*1.0/viewscale_p_m;

        //since F = G*m1*m2/(r^2)
        //F = ma
        //a2 = F/m2
        //a2 = G*m1/(r^2)
        //m1 = pi*(r^2)*d
        //a2 = G*pi*(r^2)*d/(r^2)
        //a2 = G*pi*d
        //a2 here is the maximum acceleration anything can experience when sitting on the surface of any one of my equally-dense bodies.
        float max_accel = this->big_G*M_PI*this->fixturedef_template.density;

        float accel_correction_factor = phys_min_interval/max_accel;

        //calculate accelerations
        for (int x = col_interval/2; x < this->window_size.width(); x = x + col_interval){
            for (int y = row_interval/2; y < this->window_size.height(); y = y + row_interval){

                QPoint screenStartPoint = QPoint(x, y);
                b2Vec2 worldPoint = this->scrnPtToPhysPt(screenStartPoint);

                //check to see if we are inside a body
                bool inside_body = false;
                for (b2Body* b = this->world->GetBodyList(); b; b = b->GetNext()){
                    b2Vec2 pos_diff = b->GetWorldCenter() - worldPoint;
                    float dist = pos_diff.Length();
                    if (dist < b->GetFixtureList()->GetShape()->m_radius){
                        inside_body = true;
                        break;
                    }
                }
                if (inside_body) continue;


                b2Vec2 accel = this->getAccelAt(worldPoint);

                accel *= accel_correction_factor;

                b2Vec2 vectorHeadPoint = worldPoint + accel;

                QPoint screenEndPoint = this->physPtToScrnPt(vectorHeadPoint).toPoint();

                painter.drawLine(screenStartPoint, screenEndPoint);
            }
        }
    }

    painter.setPen(Qt::SolidLine);
    painter.setPen(QColor(255, 255, 255));
    painter.setBrush(Qt::NoBrush);

    for (b2Body* b = this->world->GetBodyList(); b; b = b->GetNext()){
        this->drawBodyTo(&painter, b);
    }

}

void ABSWindow::drawBodyTo(QPainter* painter, b2Body* body){

    RETURN_IF_NULL(body);
    RETURN_IF_NULL(painter);

    painter->save();
    b2Vec2 pos = body->GetPosition();
    //printf("Body position: (%f, %f) m\n", pos.x, pos.y);

    if (this->enable_trails){
        std::vector<b2Vec2>* pos_hist = this->position_histories.value(body);

        if (pos_hist->size() > 1){
            for (uint i = 0; i < pos_hist->size()-1; ++i){
                QPointF a = this->physPtToScrnPt(pos_hist->at(i));
                QPointF b = this->physPtToScrnPt(pos_hist->at(i+1));
                painter->drawLine(a, b);
            }
        }
    }

    //printf("history size: %lu\n", pos_hist->size());

    float radius = body->GetFixtureList()->GetShape()->m_radius;

    //this is the HSV hue for light blue, which is where we want this spectrum to cap.
    float max_hue = 0.6;

    float hue = max_hue*((log10(radius)+1)/2.0); //maps 0.1 and 10 to 0 and max_hue logarithmically
    //printf("radius: %f; hue: %f\n", radius, hue);

    //cap hue to [0, max_hue]
    if (hue < 0) hue = 0;
    if (hue > max_hue) hue = max_hue;

    //reverse hue so blue is little things and red is big things
    hue = max_hue - hue;

    QColor body_color;
    body_color.setHsvF(hue, 0.8, 1.0);

    painter->setPen(body_color);
    painter->setBrush(body_color);

    QPointF body_center_px = this->physPtToScrnPt(pos);
    painter->translate(body_center_px.x(), body_center_px.y());

    ////https://stackoverflow.com/questions/8881923/how-to-convert-a-pointer-value-to-qstring
    //QString ptrStr = QString("0x%1").arg(reinterpret_cast<quintptr>(body),QT_POINTER_SIZE * 2, 16, QChar('0'));
    ////QString coordstr = QString("(%1, %2)").arg(body->GetPosition().x).arg(body->GetPosition().y);
    ////printf("\t%s: %s\n", ptrStr.toUtf8().constData(), coordstr.toUtf8().constData());

    //painter->drawText(QPoint(0, 0), ptrStr);

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

    //b2Vec2 com_m = body->GetLocalCenter();
    //painter->drawEllipse(QPointF(com_m.x, com_m.y)*this->viewscale_p_m, 10, 5);

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

b2Body* ABSWindow::createBody(float radius, b2Vec2 position, b2Vec2 velocity){

    for (b2Body* b = this->world->GetBodyList(); b; b = b->GetNext()){
        if (b->GetWorldCenter() == position){
            return nullptr;
        }
    }

    this->bodydef_template.position = position;
    this->bodydef_template.linearVelocity = velocity;
    this->circle_shape.m_radius = radius;

    b2Body* b = this->world->CreateBody(&this->bodydef_template);
    b->CreateFixture(&fixturedef_template);

    std::vector<b2Vec2>* pos_hist = new std::vector<b2Vec2>;
    this->position_histories.insert(b, pos_hist);

    emit this->numBodiesChanged(this->world->GetBodyCount());

    return b;

}

void ABSWindow::destroyBody(b2Body* b){

    this->world->DestroyBody(b);

    delete this->position_histories.value(b);
    this->position_histories.remove(b);

    emit this->numBodiesChanged(this->world->GetBodyCount());

}

void ABSWindow::doGameStep(){

    if (this->paused) return;

    std::vector<b2Body*> destroyed_bodies;

    //combine bodies that have collided
    for (collision_struct coll : this->contactlistener->collisions){

        //check if one of the bodies already got merged. if so just skip it, it will probably merge a few steps from now anyways.
        if (find(destroyed_bodies.begin(), destroyed_bodies.end(), coll.bodyA) != destroyed_bodies.end()){
            continue;
        }

        if (find(destroyed_bodies.begin(), destroyed_bodies.end(), coll.bodyB) != destroyed_bodies.end()){
            continue;
        }

        float totalmass = coll.bodyA->GetMass();
        totalmass += coll.bodyB->GetMass();
        b2Vec2 totalmomentum = coll.bodyA->GetMass()*coll.bodyA->GetLinearVelocity();
        totalmomentum += coll.bodyB->GetMass()*coll.bodyB->GetLinearVelocity();

        b2Vec2 totalvelocity = (1.0f/totalmass)*totalmomentum;

        float totalarea = totalmass/this->fixturedef_template.density;
        float totalradius = sqrt(totalarea/M_PI);

        b2Vec2 totalpos = (1.0f/totalmass)*(coll.bodyA->GetMass()*coll.bodyA->GetWorldCenter() + coll.bodyB->GetMass()*coll.bodyB->GetWorldCenter());

        this->createBody(totalradius, totalpos, totalvelocity);

        this->destroyBody(coll.bodyA);
        destroyed_bodies.push_back(coll.bodyA);

        this->destroyBody(coll.bodyB);
        destroyed_bodies.push_back(coll.bodyB);

    }

    this->contactlistener->collisions.clear();

    //apply gravity
    for (b2Body* b = this->world->GetBodyList(); b; b = b->GetNext()){

        b2Vec2 Force = b->GetMass()*this->getAccelAt(b->GetWorldCenter(), b);
        //b->ApplyLinearImpulseToCenter(Force, true);
        b->ApplyForceToCenter(Force, true);

        //TODO: preserve trail histories (to a point) when bodies merge?
        std::vector<b2Vec2>* pos_hist = this->position_histories.value(b);
        pos_hist->push_back(b->GetWorldCenter());

        if (pos_hist->size() > this->max_position_hist_entries){
            pos_hist->erase(pos_hist->begin(), pos_hist->end() - this->max_position_hist_entries);
        }
    }

    int velocityIterations = 8;
    int positionIterations = 3;
    this->world->Step(this->timeStep_s, velocityIterations, positionIterations);

}

b2Vec2 ABSWindow::getAccelAt(b2Vec2 worldPoint, b2Body* exception_body){

    b2Vec2 AccelAccum = b2Vec2(0, 0);

    for (b2Body* b = this->world->GetBodyList(); b; b = b->GetNext()){

        if (b == exception_body) continue;

        b2Vec2 vect_to_ob = b->GetWorldCenter() - worldPoint;

        float accel_mag = big_G*b->GetMass()/vect_to_ob.LengthSquared();

        b2Vec2 accel_to_add = vect_to_ob;
        accel_to_add.Normalize();
        accel_to_add *= accel_mag;

        AccelAccum += accel_to_add;
    }
    return AccelAccum;
}
