/*
 * Copyright 2016 Stoned Xander
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef HEADLESS_LOGIC_NEURAL_NETWORK
#define HEADLESS_LOGIC_NEURAL_NETWORK

#include <cstring>

namespace Headless {
    namespace Logic {
        namespace NeuralNet {

            /**
             * Simple Mono-Layer Recursive Neural-Net.
             * Implementation of the core-engine of a lifetime
             * home-brew neural net scheme.
             * (Input(t), Output(t), Intermediate(t)) -> F -> (Output(t+1), Intermediate(t+1))
             * @param <F> The activation function. There's only
             *    one per neural-net in this case.
             *    An activation function must define the following:
             *      F(inputSignalCount)
             *      double compute(double *);
             */
            template <typename F> class MonoRecursive {
                public:
                    /**
                     * Constructor.
                     * @param input Number of input signals.
                     * @param output Number of output signals.
                     * @param intermediate Number of intermediate neurons.
                     */
                    MonoRecursive(unsigned int input, unsigned int output, unsigned int intermediate);

                    /**
                     * Destructor.
                     */
                    ~MonoRecursive();

                    /**
                     * Retrieve a neuron.
                     * @param index Index of the neuron to retrieve.
                     * @return The neuron.
                     */
                    F* operator[](unsigned int index) {
                        F* result = nullptr;
                        if(index < (output + intermediate)) {
                            result = _layer[i];
                        }
                        return result;
                    }

                    /**
                     * Perform one computation step.
                     * @param input Input vector (containing numbers in [0;1])
                     * @param output Result vector (containing numbers in [0;1])
                     */
                    void compute(double *input, double *output);

                private:
                    /**
                     * Input array.
                     */
                    double *_input;

                    /**
                     * Output array.
                     */
                    double *_output;

                    /**
                     * Activation function layer.
                     */
                    F **_layer;

                    /**
                     * Input signal count.
                     */
                    unsigned int _inCount;

                    /**
                     * Output signal count.
                     */
                    unsigned int _outCount;

                    /**
                     * Intermediate neurons count.
                     */
                    unsigned int _mediumCount;
            };

            MonoRecursive::MonoRecursive(unsigned int input, unsigned int output, unsigned int intermediate) :
                _input(new double[input + output + intermediate]),
                _output(new double[output + intermediate]),
                _layer(new F*[output + intermediate]),
                _inCount(input), _outCount(output), _mediumCount(intermediate) {
                unsigned int size = output + intermediate;
                for(unsigned int i = 0; i < size; ++i) {
                    _layer[i] = new F(input + output + intermediate);
                }
            }

            void MonoRecursive::compute(double *input, double *output) {
                memcpy(_input, input, _inCount);
                unsigned int size = _outCount + _mediumCount;
                memcpy(_input + _inCount, _output, size);
                #pragma openmp parallel for
                for(unsigned int i = 0; i < size; ++i) {
                    _output[i] = _layer[i]->compute(_input);
                }
                memcpy(output, _output, _outCount);
            }

            MonoRecursive::~MonoRecursive() {
                delete[] _input;
                delete[] _output;
                unsigned int size = _outCount + _mediumCount;
                for(unsigned int i = 0; i < size; ++i) {
                    delete _layer[i];
                }
                delete[] _layer;
            }

        } // Namespace 'NeuralNet'
    } // Namespace 'Logic'
} // Namespace 'Headless'

#endif
