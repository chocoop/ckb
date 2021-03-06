#include "kbmode.h"
#include "kb.h"

KbMode::KbMode(Kb* parent, const KeyMap& keyMap, const QString &guid, const QString& modified) :
    QObject(parent),
    _name("Unnamed"), _id(guid, modified),
    _light(new KbLight(this, keyMap)), _bind(new KbBind(this, parent, keyMap)),
    _needsSave(true)
{
    connect(_light, SIGNAL(updated()), this, SLOT(doUpdate()));
    if(_id.guid.isNull())
        _id.guid = QUuid::createUuid();
}

KbMode::KbMode(Kb* parent, const KeyMap& keyMap, const KbMode& other) :
    QObject(parent),
    _name(other._name), _id(other._id),
    _light(new KbLight(this, keyMap, *other._light)), _bind(new KbBind(this, parent, keyMap, *other._bind)),
    _needsSave(true)
{
    connect(_light, SIGNAL(updated()), this, SLOT(doUpdate()));
}

KbMode::KbMode(Kb *parent, const KeyMap &keyMap, QSettings &settings) :
    QObject(parent),
    _name(settings.value("Name").toString().trimmed()),
    _id(settings.value("GUID").toString().trimmed(), settings.value("Modified").toString().trimmed()),
    _light(new KbLight(this, keyMap)), _bind(new KbBind(this, parent, keyMap)),
    _needsSave(false)
{
    connect(_light, SIGNAL(updated()), this, SLOT(doUpdate()));
    if(_id.guid.isNull())
        _id.guid = QUuid::createUuid();
    if(_name == "")
        _name = "Unnamed";
    _light->load(settings);
    _bind->load(settings);
}

void KbMode::newId(){
    _needsSave = true;
    _id = UsbId();
    // Create new IDs for animations
    foreach(KbAnim* anim, _light->animList())
        anim->newId();
}

void KbMode::keyMap(const KeyMap &keyMap){
    _needsSave = true;
    _light->map(keyMap);
    _bind->map(keyMap);
}

void KbMode::save(QSettings& settings){
    _needsSave = false;
    settings.setValue("GUID", _id.guidString());
    settings.setValue("Modified", _id.modifiedString());
    settings.setValue("Name", _name);
    _light->save(settings);
    _bind->save(settings);
}

bool KbMode::needsSave() const {
    return _needsSave || _light->needsSave() || _bind->needsSave();
}

void KbMode::doUpdate(){
    emit updated();
}
