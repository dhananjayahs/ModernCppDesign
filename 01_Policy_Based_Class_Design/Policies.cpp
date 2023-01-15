#include <cstdlib>

class Widget {};
class Gadget {};

template <class T> struct OpNewCreator {
  static T Create() { return new T; }

protected:
  ~OpNewCreator() {}
};

template <class T> struct MallocCreator {
  static T Create() {
    void *buf = std::malloc(sizeof(T));
    if (!buf)
      return 0;
    return new (buf) T;
  }

protected:
  ~MallocCreator() {
  } // All policy classes need to have protected non-virtual dtors. So the host
    // classes can't be deleted with a policy pointer to host class.
};

template <class T> struct PrototypeCreator {
  PrototypeCreator(T *pObj = 0) : pPrototype_(pObj) {}

  T *Create() { return pPrototype_ ? pPrototype_->Clone() : 0; }

  T *GetPrototype() { return pPrototype_; }
  void SetPrototype(T *pObj = 0) { pPrototype_ = pObj; }

protected:
  ~PrototypeCreator() {}

private:
  T *pPrototype_;
};

// Library Code
template <template <class Created> class CreationPolicy =
              OpNewCreator> // Created can be omitted
class WidgetManager : public CreationPolicy<Widget> {
public:
  void DoSomething() {
    // due to template template parameters we can also use the same creation
    // policy for gadgets here
    Gadget *pw = CreationPolicy<Gadget>().Create();
    (void)pw; // just as demonstration, not used
  }

  void SwitchPrototype(Widget *pNewPrototype) {
    CreationPolicy<Widget> &myPolicy = *this;
    delete myPolicy.GetPrototype();
    myPolicy.SetPrototype(pNewPrototype);
  }

private:
};
/* You cannot use Created inside WidgetManagerit is a formal argument for
   CreationPolicy (not WidgetManager) and can be simply omitted . */

// Application code
typedef WidgetManager<MallocCreator> MyWidgetManager;

int main() {
  WidgetManager<> widgetManager1;   // default argument for class template which is OpNewCreator in this case
  MyWidgetManager *widgetManager2 = new MyWidgetManager();
  MallocCreator<Widget> *castedWidgetManager2 =
      dynamic_cast<MallocCreator<Widget> *>(widgetManager2);
  (void)castedWidgetManager2; // not possible to delete when dtor of
                              // MallocCreator is protected: delete
                              // castedWidgetManager2;

  // but this works
  delete widgetManager2;

  WidgetManager<PrototypeCreator> widgetManager3;
  Widget *newPrototype = new Widget();
  widgetManager3.SwitchPrototype(newPrototype);

  // This doesn't work: widgetManager1.SwitchPrototype(newPrototype);

  return 0;
}
