#ifndef ABSWINDOW_H
#define ABSWINDOW_H

#include <cmath>

#include <QPainterPath>

#include "box2d/box2d.h"

#include <opengl2dwindow.h>


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


    QSize window_size;


    //scaled up to increase scale of stuff
    //normally is *10^-11
    const float big_G = 6.6743015*pow(10, -1); //N*m^2/(kg^2)

    b2World* world = nullptr;

    float timeStep_s;

    b2Vec2 viewcenter_m = b2Vec2(0, 0);
    float viewscale_p_m = 1;

};

#endif // ABSWINDOW_H
