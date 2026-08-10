#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <queue>
#include <typeindex>
#include <core/logger.hpp>

namespace Core::Events {
  enum class EventType {
    None = 0,
    QuitApplication,

    MouseMotion,

    WindowResized,
    WindowShown,
    WindowHidden,
    WindowMouseEnter,
    WindowMouseLeave,
    WindowFocusGained,
    WindowFocusLost,
    WindowMinimized,
    WindowMaximized,
    WindowRestored,
    WindowClosed,
    WindowExposed,

    KeyPressed,
    KeyReleased
  };

  class Event {
    public:
      explicit Event(EventType type, const char *name) : name(name), type(type) {
      }

      virtual ~Event() = default;

      const EventType type;
      bool handled = false;
      std::uint64_t time = 0;
      const char *name = "Event";
  };

  class EventDispatcher {
    using ListenerHandle = std::uint64_t;
    using Listeners = std::map<ListenerHandle, std::function<void(const void *)> >;

    public:
      EventDispatcher() = default;

      EventDispatcher(EventDispatcher &&) noexcept = default;

      EventDispatcher(const EventDispatcher &) = delete;

      EventDispatcher &operator=(EventDispatcher &&) noexcept = default;

      template<typename T>
      static std::uint64_t listen(const std::function<void(const T &)> &listener) {
        auto &eventListeners = listeners[std::type_index(typeid(T))];
        const auto listenerHandle = ListenerHandle{listenerCount++};

        eventListeners[listenerHandle] = [listener](const auto &msg) {
          const T *concreteMessage = static_cast<const T*>(msg);
          listener(*concreteMessage);
        };
        return listenerHandle;
      }

      template<typename EventT>
      static void queue(EventT &&event) {
        queuedDispatches.push([m = std::forward<EventT>(event)](EventDispatcher &dispatcher) {
          dispatcher.dispatch(m);
        });
      }

      static void removeListener(const std::uint64_t handle) {
        for (auto &listeners: listeners) {
          listeners.second.erase(handle);
        }
      }

      void process() {
        while (!queuedDispatches.empty()) {
          std::function<void(EventDispatcher&)> queuedDispatch = std::move(queuedDispatches.front());
          queuedDispatches.pop();
          queuedDispatch(*this);
        }
      }

    private:
      template<typename EventT>
      void dispatch(const EventT &msg) {
        const auto &listenersIter = listeners.find(std::type_index(typeid(EventT)));
        if (listenersIter == listeners.end()) {
          return; // No listeners for this type of message
        }

        const auto &[msgType, listeners] = *listenersIter;

        // Cache handles to avoid iterator invalidation. This way listeners can safely remove themselves.
        std::vector<ListenerHandle> handles;
        handles.reserve(listeners.size());
        for (auto &[handle, listener]: listeners) {
          handles.push_back(handle);
        }

        for (auto &handle: handles) {
          const auto &handleAndListener = listeners.find(handle);
          const bool listenerPresent = handleAndListener != listeners.end();
          if (listenerPresent) {
            const auto &listener = handleAndListener->second;
            listener(&msg);
          }
        }
      }

      static inline std::map<std::type_index, Listeners> listeners;
      static inline std::queue<std::function<void(EventDispatcher&)>> queuedDispatches;

      static inline std::uint64_t listenerCount;
  };
}
