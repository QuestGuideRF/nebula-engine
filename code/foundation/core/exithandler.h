#pragma once
namespace Core
{
class ExitHandler
{
public:
    ExitHandler();
    virtual ~ExitHandler();
    virtual void OnExit() const;
    const ExitHandler* Next() const;
private:
    const ExitHandler* nextExitHandler;
};
}