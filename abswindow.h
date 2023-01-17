#ifndef ABSWINDOW_H
#define ABSWINDOW_H

#include <cmath>

#include <QPainterPath>
#include <QResizeEvent>
#include <QHash>

#include "box2d/box2d.h"

#include <opengl2dwindow.h>
#include <abscontactlistener.h>


#define MILLIS_PER_SECOND (1000)
#define MICROS_PER_SECOND (1000000)
#define NANOS_PER_SECOND  (1000000000)

#define DEG_PER_RAD (57.295779513082321)

//semicolon OK after these macros
#define RETURN_VAL_IF_COND(cond, msg, ret_val) \
    if (cond){ \
        fprintf(stderr, "%s called with %s\n", __FUNCTION__, msg); \
        return ret_val; \
    }

#define RETURN_IF_COND(cond, msg) RETURN_VAL_IF_COND(cond, msg, )

#define RETURN_VAL_IF_COND_NOPRINT(cond, ret_val) \
    if (cond){ \
        return ret_val; \
    }

#define RETURN_IF_COND_NOPRINT(cond) RETURN_VAL_IF_COND_NOPRINT(cond, )

#define RETURN_VAL_IF_FLT_INVALID(flt, ret_val) \
    if (isnan(flt) || isinf(flt)){ \
        fprintf(stderr, "%s called with invalid %s: %f\n", __FUNCTION__, #flt, flt); \
        return ret_val; \
    }

#define RETURN_IF_FLT_INVALID(flt) RETURN_VAL_IF_FLT_INVALID(flt, )

#define RETURN_VAL_IF_NULL(ptr, ret_val) \
    if (!ptr){ \
        fprintf(stderr, "%s called with null %s\n", __FUNCTION__, #ptr); \
        return ret_val; \
    }

#define RETURN_VAL_IF_NULL_NOPRINT(ptr, ret_val) \
    if (!ptr) return ret_val

#define RETURN_IF_NULL(ptr) RETURN_VAL_IF_NULL(ptr, )

#define RETURN_IF_NULL_NOPRINT(ptr) RETURN_VAL_IF_NULL_NOPRINT(ptr, )

class ABSWindow : public OpenGL2DWindow
{
public:
    ABSWindow();
    ~ABSWindow() override;

    void setupWindow();

    void render(QPainter &painter) override;

    void doGameStep() override;

    void drawBodyTo(QPainter* painter, b2Body* body);

    QPointF physPtToScrnPt(b2Vec2 worldPoint_m);

    b2Vec2 scrnPtToPhysPt(QPointF screenPoint_p);

    b2Body* createBody(float radius, b2Vec2 position, b2Vec2 velocity);

    void destroyBody(b2Body* b);


    QSize window_size;

    bool mouse_down_create_body = false;
    bool mouse_down_drag_view = false;

    QPointF start_click_pos;
    QPointF end_click_pos;


    //scaled up to increase scale of stuff
    //normally is 6.6743015*10^-11
    const float big_G = 6.6743015*pow(10, 0); //N*m^2/(kg^2)

    b2World* world = nullptr;
    ABSContactListener* contactlistener = nullptr;

    float timeStep_s;

    b2Vec2 viewcenter_m = b2Vec2(0, 0);
    float viewscale_p_m = 10;

    b2BodyDef bodydef_template;
    b2FixtureDef fixturedef_template;
    b2CircleShape circle_shape;

    QHash<b2Body*, std::vector<b2Vec2>*> position_histories;
    uint max_position_hist_entries = 100;

protected:

    void resizeEvent(QResizeEvent* event) override;

    void mousePressEvent(QMouseEvent* ev) override;

    void mouseReleaseEvent(QMouseEvent* ev) override;

    void mouseMoveEvent(QMouseEvent* ev) override;

    void wheelEvent(QWheelEvent *ev) override;

};

#endif // ABSWINDOW_H
