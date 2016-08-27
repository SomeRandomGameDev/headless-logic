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
             * Trivial mono-layer recursive neural-net.
             * (Input(t), Output(t), Intermediate(t)) -> F -> (Output(t+1), Intermediate(t+1))
             */
            template class TrivialMonoRecursive {
                public:
                    /**
                     * Constructor.
                     * @param input Number of input signals.
                     * @param output Number of output signals.
                     * @param intermediate Number of intermediate neurons.
                     */
                    TrivialMonoRecursive(unsigned int input, unsigned int output, unsigned int intermediate);

                    /**
                     * Destructor.
                     */
                    ~TrivialMonoRecursive();

                    /**
                     * Perform one computation step.
                     * @param input Input vector (containing numbers in [0;1])
                     * @param output Result vector (containing numbers in [0;1])
                     * @param function Activation function.
                     * @param <F> Activation function type. It must implement the following:
                     *      double compute(double inValue, double bias);
                     */
                    template <F> void compute(double *input, double *output, F *function);

                    double *weights() { return _weight; }

                    double *biases() { return _bias; }

                private:
                    /**
                     * Input signals.
                     */
                    double *_input;

                    /**
                     * Output signals.
                     */
                    double *_output;

                    /**
                     * Weights.
                     */
                    double *_weight;

                    /**
                     * Biases.
                     */
                    double *_bias;

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


            TrivialMonoRecursive::TrivialMonoRecursive(
                    unsigned int input, unsigned int output, unsigned int intermediate) :
                _input(new double[input + output + intermediate]),
                _output(new double[output + intermediate]),
                _weight(new double[(input + output + intermediate) * (output + intermediate)]),
                _biases(new double[output + intermediate]),
                _inCount(input), _outCount(output), _mediumCount(intermediate) {
                }

            TrivialMonoRecursive::~TrivialMonoRecursive() {
                delete[] _input;
                delete[] _output;
                delete[] _weigth;
                delete[] _bias;
            }

            template <typename F>
                void TrivialMonoRecursive::compute(double* inSig, double* outSig, F* function) {
                    memcpy(_input, inSig, _inCount);
                    const unsigned int size = _outCount + _mediumCount;
                    const unsigned int inSize = _inCount + size;
                    memcpy(_input + _inCount, _output, size);
#pragma openmp parallel for
                    for(unsigned int i = 0; i < size; ++i) {
                        double inValue = 0;
                        double *inW = _weigth + (i * inSize);
                        for(unsigned int j = 0; j < inSize; ++j) {
                            inValue += _input[j] * inW[j];
                        }
                        _output[i] = function->compute(inValue, _bias[i]);
                    }
                    memcpy(outSig, _output, _outCount);
                }

            /**
             * Simple Mono-Layer Recursive Neural-Net.
             * Implementation of the core-engine of a lifetime
             * home-brew neural net scheme.
             * (Input(t), Output(t), Intermediate(t)) -> F -> (Output(t+1), Intermediate(t+1))
             * @param <F> The activation function. There's only
             *    one per neural-net in this case.
             *    An activation function must define the following:
             *      F(neuronId, inputSignalCount, args...)
             *      double compute(double *);
             */
            template <typename F> class MonoRecursive {
                public:
                    /**
                     * Constructor.
                     * @param input Number of input signals.
                     * @param output Number of output signals.
                     * @param intermediate Number of intermediate neurons.
                     * @þaram <A...> List of arguments type for neurons initialisation.
                     * @param args... Arguments list for neuron initialisation.
                     */
                    template<typename A...>
                        MonoRecursive(unsigned int input, unsigned int output, unsigned int intermediate
                                A args...);

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

            template <typename F>
                template <typename A...>
                MonoRecursive<F>::MonoRecursive(
                        unsigned int input, unsigned int output, unsigned int intermediate,
                        A args...) :
                    _input(new double[input + output + intermediate]),
                    _output(new double[output + intermediate]),
                    _layer(new F*[output + intermediate]),
                    _inCount(input), _outCount(output), _mediumCount(intermediate) {
                        unsigned int size = output + intermediate;
                        for(unsigned int i = 0; i < size; ++i) {
                            _layer[i] = new F(i, input + output + intermediate, args...);
                        }
                    }

            template <typename F>
                void MonoRecursive<F>::compute(double *input, double *output) {
                    memcpy(_input, input, _inCount);
                    unsigned int size = _outCount + _mediumCount;
                    memcpy(_input + _inCount, _output, size);
#pragma openmp parallel for
                    for(unsigned int i = 0; i < size; ++i) {
                        _output[i] = _layer[i]->compute(_input);
                    }
                    memcpy(output, _output, _outCount);
                }

            template <typename F>
                MonoRecursive<F>::~MonoRecursive() {
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
