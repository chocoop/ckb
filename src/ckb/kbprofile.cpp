#include "kbprofile.h"
#include "kb.h"

KbProfile::KbProfile(Kb* parent, const KeyMap& keyMap, const KbProfile& other) :
    QObject(parent), _currentMode(0), _name(other._name), _id(other._id), _keyMap(keyMap), _needsSave(true)
{
    foreach(KbMode* mode, other.modes()){
        KbMode* newMode = new KbMode(parent, keyMap, *mode);
        if(!_currentMode || mode == other._currentMode)
            _currentMode = newMode;
        _modes.append(newMode);
    }
}

KbProfile::KbProfile(Kb* parent, const KeyMap& keyMap, const QString& guid, const QString& modified) :
    QObject(parent), _currentMode(0), _id(guid, modified.toUInt(0, 16)), _keyMap(keyMap), _needsSave(true)
{
    if(_id.guid.isNull())
        _id.guid = QUuid::createUuid();
}

KbProfile::KbProfile(Kb* parent, const KeyMap& keyMap, QSettings& settings, const QString& guid) :
    QObject(parent), _currentMode(0), _id(guid, 0), _keyMap(keyMap), _needsSave(false)
{
    // Load data from preferences
    settings.beginGroup(guid);
    _name = settings.value("Name").toString().trimmed();
    if(_name == "")
        _name = "Unnamed";
    _id.modifiedString(settings.value("Modified").toString());
    QUuid current = settings.value("CurrentMode").toString().trimmed();
    // Load modes
    uint count = settings.value("ModeCount").toUInt();
    for(uint i = 0; i < count; i++){
        settings.beginGroup(QString::number(i));
        KbMode* mode = new KbMode(parent, _keyMap, settings);
        _modes.append(mode);
        // Set currentMode to the mode matching the current GUID, or the first mode in case it's not found
        if(current == mode->id().guid || !_currentMode)
            _currentMode = mode;
        settings.endGroup();
    }
    settings.endGroup();
}

void KbProfile::save(QSettings& settings){
    _needsSave = false;
    // Save data to preferences
    settings.beginGroup(id().guidString());
    settings.setValue("Name", name());
    settings.setValue("Modified", _id.modifiedString());
    if(_currentMode)
        settings.setValue("CurrentMode", _currentMode->id().guidString());
    // Save modes
    uint count = modeCount();
    settings.setValue("ModeCount", count);
    for(uint i = 0; i < count; i++){
        settings.beginGroup(QString::number(i));
        KbMode* mode = _modes.at(i);
        mode->save(settings);
        settings.endGroup();
    }
    settings.endGroup();
}

bool KbProfile::needsSave() const {
    if(_needsSave)
        return true;
    foreach(KbMode* mode, _modes){
        if(mode->needsSave())
            return true;
    }
    return false;
}

void KbProfile::newId(){
    _needsSave = true;
    _id = UsbId();
    foreach(KbMode* mode, _modes)
        mode->newId();
}

void KbProfile::keyMap(const KeyMap& newKeyMap){
    _keyMap = newKeyMap;
    foreach(KbMode* mode, _modes)
        mode->keyMap(newKeyMap);
}
