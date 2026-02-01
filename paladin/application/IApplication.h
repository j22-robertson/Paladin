//
// Created by James Robertson on 29/01/2026.
//

#ifndef PALADIN_IAPPLICATION_H
#define PALADIN_IAPPLICATION_H
class IApplication {
public:
    IApplication() = default;
    virtual ~IApplication() = default;
    virtual void run() = 0;
    virtual void Setup() = 0;
    virtual bool Update(float delta_time){return true;};
    virtual void Render(float delta_time)=0;
private:

protected:

};



#endif //PALADIN_IAPPLICATION_H