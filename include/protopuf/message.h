#ifndef PROTOPUF_MESSAGE_H
#define PROTOPUF_MESSAGE_H

#include "coder.h"
#include "field.h"
#include "float.h"

namespace pp {

    template <field_c ... T>
    struct message : private T... {

        message() = default;

        explicit message(T&& ...v) : T(std::forward<T>(v))... {};

        message(const message& other) : T(static_cast<const T&>(other))... {}

        static constexpr uint<4> size = sizeof...(T);

        template <uint<4> N>
        constexpr decltype(auto) get() const {
            return static_cast<const field_selector<N, T...>&>(*this);
        }

        template <uint<4> N>
        constexpr decltype(auto) get() {
            return static_cast<field_selector<N, T...>&>(*this);
        }

        template <uint<4> N>
        constexpr decltype(auto) get_base() const {
            return static_cast<const typename field_selector<N, T...>::base_type&>(get<N>());
        }

        template <uint<4> N>
        constexpr decltype(auto) get_base() {
            return static_cast<typename field_selector<N, T...>::base_type&>(get<N>());
        }

        constexpr bool operator==(const message & other) const {
            return ((
                    static_cast<const typename T::base_type&>(static_cast<const T&>(*this)) ==
                    static_cast<const typename T::base_type&>(static_cast<const T&>(other))
                    ) && ...);
        }

        constexpr bool operator!=(const message & other) const {
            return !(*this == other);
        }

        template <typename F> requires (std::invocable<F, T> && ...)
        constexpr void for_each(F&& f) const {
            (std::forward<F>(f)(static_cast<const T&>(*this)), ...);
        }

        template <typename F> requires (std::invocable<F, T> && ...)
        constexpr void for_each(F&& f) {
            (std::forward<F>(f)(static_cast<T&>(*this)), ...);
        }

    };

    template <typename>
    constexpr bool is_message = false;

    template <typename ...T>
    constexpr bool is_message <message<T...>> = true;

    template <typename T>
    concept message_c = is_message<T>;

    template <message_c>
    struct message_decode_map;

    template <field_c... F>
    struct message_decode_map<message<F...>> : std::unordered_map<uint<4>, std::function<bytes(message<F...>&, bytes)>> {
    private:
        using T = message<F...>;

    public:
        message_decode_map() : std::unordered_map<uint<4>, std::function<bytes(T&, bytes)>> {
                {F::key, [](T& m, bytes b){
                    const auto &[v, np] = F::coder::decode(b);

                    auto &f = m.template get<F::number>();
                    if constexpr (F::attr == singular) {
                        f = v;
                    } else {
                        *std::inserter(f, f.end()) = v;
                    }

                    return np;
                }}...
        } {}
    };

    template <message_c T>
    struct message_coder {
        using value_type = T;

        message_coder() = delete;

        static constexpr bytes encode(const T& msg, bytes b) {
            msg.for_each([&b]<field_c F> (const F& f) {
                if(empty_field(f)) {
                    return;
                }


                if constexpr (F::attr == singular) {
                    b = varint_coder<uint<4>>::encode(F::key, b);
                    b = F::coder::encode(f.value(), b);
                } else {
                    for(const auto &i : f) {
                        b = varint_coder<uint<4>>::encode(F::key, b);
                        b = F::coder::encode(i, b);
                    }
                }
            });

            return b;
        }

        static inline const message_decode_map<T> decode_map;

        static constexpr decode_result<T> decode(bytes b) {
            T v;

            auto iter = decode_map.end();
            while(true) {
                const auto &[n, nb] = varint_coder<uint<4>>::decode(b);

                iter = decode_map.find(n);
                if (iter != decode_map.end()) {
                    b = nb;
                    b = iter->second(v, b);
                } else {
                    break;
                }
            }

            return {v, b};
        }
    };
}

#endif //PROTOPUF_MESSAGE_H
