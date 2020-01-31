#ifndef QISCRIPTENGINECLUSTER
#define QISCRIPTENGINECLUSTER

#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QTimer>
#include <QList>

class QiScriptEngineCluster : public QScriptEngine
{
    Q_OBJECT

public:
    explicit QiScriptEngineCluster(){
        MainEngine = new QScriptEngine;
        //QTimer *timer = new QTimer;
        //connect(timer,SIGNAL(timeout()),this,SLOT(clearFreeEngine()));
        //timer->start(500);
    }

    QScriptValue globalObject(){
        return MainEngine->globalObject();
    }

    QScriptValue newArray(uint length = 0){
        return MainEngine->newArray(length);
    }

    QScriptValue evaluate(const QScriptProgram &program){
        if(!MainEngine->isEvaluating()){
            return MainEngine->evaluate(program);
        }
        else{
            return freeEngine()->evaluate(program);
        }
        return QScriptValue::UndefinedValue;
    }

    QScriptValue evaluate(QString program){
        if(!MainEngine->isEvaluating()){
            return MainEngine->evaluate(program);
        }
        else{
            return freeEngine()->evaluate(program);
        }
        return QScriptValue::UndefinedValue;
    }

    QScriptValue evaluate_M(const QScriptProgram &program){
        return MainEngine->evaluate(program);
    }

    QScriptValue evaluate_M(QString program){
        return MainEngine->evaluate(program);
    }

    QScriptValue call(QScriptString str, QScriptValueList arg){
        return freeEngine()->globalObject().property(str).call(QScriptValue(),arg);
    }

    ~QiScriptEngineCluster(){
        clearSubEngine();
        delete MainEngine;
    }

public slots:
    void clearFreeEngine(){
        for(int i=0; i<SubEngine.size(); i++){
            if(!SubEngine.at(i)->isEvaluating()){
                delete SubEngine.at(i);
                SubEngine.removeAt(i);
                i--;
            }
        }
    }

    void clearSubEngine(){
        for(int i=0; i<SubEngine.size(); i++)delete SubEngine.at(i);
        SubEngine.clear();
    }

private:
    QScriptEngine* freeEngine(){
        for(int i=0; i<SubEngine.size();i++){
            if(!SubEngine.at(i)->isEvaluating()){
                return SubEngine.at(i);
            }
        }
        return newEngine();
    }

    QScriptEngine* newEngine(){
        QScriptEngine *e = new QScriptEngine;
        QScriptValueIterator it(MainEngine->globalObject());
        while(it.hasNext()){
            it.next();
            QString name = it.name();
            QScriptValue value = it.value();
            if(value.isNumber())e->globalObject().setProperty(name,e->evaluate(value.toString()));
            else if(value.isFunction()){
                QString code = value.toString();
                if(!code.contains("native"))e->globalObject().setProperty(name,e->evaluate("("+code+")"));
            }
            else if(value.isArray()){
                QString array = value.toString();
                array.remove('"');
                array.prepend('[');
                array.append(']');
                e->globalObject().setProperty(name,e->evaluate(array));
            }
        }
        SubEngine.append(e);
        return e;
    }

    QScriptEngine *MainEngine;
    QList<QScriptEngine*> SubEngine;
};

#endif // QISCRIPTENGINECLUSTER

