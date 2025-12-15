#pragma once
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>

namespace Widgets {
class MapErropView : public QWidget {
    Q_OBJECT;

   private:
    QLabel* errorMassage = nullptr;
    QDoubleSpinBox* laPicker = nullptr;
    QDoubleSpinBox* loPicker = nullptr;
    QFormLayout* pickersLayout = nullptr;

   public:
    MapErropView(/* args */);
    ~MapErropView();
    void setErrorMassage(const QString& string);
    void setVisiblePickers(bool val);
    void setPickerValues(double la, double lo);
   signals:
    void inputChanged(double la, double lo);
};
}  // namespace Widgets
