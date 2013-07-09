#ifndef IVW_PROCESSORLISTWIDGET_H
#define IVW_PROCESSORLISTWIDGET_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QTreeWidget>
#include <QDrag>

#include <inviwo/qt/editor/inviwodockwidget.h>

#include <inviwo/core/processors/processorfactoryobject.h>

namespace inviwo {

class IVW_QTEDITOR_API ProcessorList : public QListWidget {

public:
    ProcessorList(QWidget* parent) : QListWidget(parent) {};
    ~ProcessorList() {};

protected:
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

private:
    QPoint dragStartPosition_;
};

class IVW_QTEDITOR_API ProcessorListWidget : public InviwoDockWidget {
Q_OBJECT
public:
    ProcessorListWidget(QWidget* parent);
    ~ProcessorListWidget();

private:
    ProcessorList* processorList_;
    QPoint dragStartPosition_;

    bool processorFits(ProcessorFactoryObject* processor, const QString& filter);

private slots:
    void addProcessorsToList(const QString& text="");
};

class IVW_QTEDITOR_API ProcessorTree : public QTreeWidget {

public:
    ProcessorTree(QWidget* parent) : QTreeWidget(parent) {};
    ~ProcessorTree() {};

protected:
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

private:
    QPoint dragStartPosition_;
};

class IVW_QTEDITOR_API ProcessorTreeWidget : public InviwoDockWidget {
    Q_OBJECT
public:
    ProcessorTreeWidget(QWidget* parent);
    ~ProcessorTreeWidget();

private:
    ProcessorTree* processorTree_;
    QPoint dragStartPosition_;

    bool processorFits(ProcessorFactoryObject* processor, const QString& filter);
    QIcon* getCodeStateIcon(Processor::CodeState);

private slots:
    void addProcessorsToTree(const QString& text="");

private:
    QIcon iconStable_;
    QIcon iconExperimental_;
    QIcon iconBroken_;
};

class IVW_QTEDITOR_API ProcessorDragObject : public QDrag {
public:
    ProcessorDragObject(QWidget* source, const QString className);

    static bool canDecode(const QMimeData* mimeData);
    static bool decode(const QMimeData* mimeData, QString& className);
};

} // namespace

#endif // IVW_PROCESSORLISTWIDGET_H