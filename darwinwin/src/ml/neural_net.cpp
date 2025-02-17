#include "neural_net.h"
#include "testable.h"
#include "io.h"

REGISTER_TESTABLE_FILE(0)

DEFINE_TESTABLE(neural_net_bias_test)
{
  lsResult result = lsR_Success;

  constexpr size_t input_layer_block_count = 1;
  constexpr size_t output_layer_block_count = 2;
  neural_net<input_layer_block_count, output_layer_block_count> nn;
  decltype(nn)::io_buffer_t io;

  constexpr size_t expected_out_count = output_layer_block_count * neural_net_block_size;
  constexpr size_t actual_out_count = LS_ARRAYSIZE(io.data);
  static_assert(actual_out_count == expected_out_count);

  lsZeroMemory(&nn);

  const int8_t values[] = { 0, 127, -128, 64, -64 };

  for (size_t i = 0; i < 5; i++)
  {
    nn.data.biases[i * 5 + 1] = values[1];
    nn.data.biases[i * 5 + 2] = values[2];
    nn.data.biases[i * 5 + 3] = values[3];
    nn.data.biases[i * 5 + 4] = values[4];
  }

  for (size_t i = 0; i < 5; i++)
  {
    nn.data.weights[i * input_layer_block_count * neural_net_block_size + 0] = values[i];
    nn.data.weights[i * input_layer_block_count * neural_net_block_size + 1] = values[i];
    nn.data.weights[i * input_layer_block_count * neural_net_block_size + 2] = values[i];
    nn.data.weights[i * input_layer_block_count * neural_net_block_size + 3] = values[i];
    nn.data.weights[i * input_layer_block_count * neural_net_block_size + 4] = values[i];
  }

  for (size_t i = 0; i < LS_ARRAYSIZE(io.data); i++)
    io[i] = lsMaxValue<int8_t>();

  neural_net_eval(nn, io);

  constexpr int8_t max = lsMaxValue<int8_t>();
  constexpr int8_t min = lsMinValue<int8_t>();

  TESTABLE_ASSERT_EQUAL(io[0], 0);
  TESTABLE_ASSERT_EQUAL(io[1], max);
  TESTABLE_ASSERT_EQUAL(io[2], min);
  TESTABLE_ASSERT_EQUAL(io[3], max);
  TESTABLE_ASSERT_EQUAL(io[4], min);

  TESTABLE_ASSERT_EQUAL(io[5 + 0], values[0]);
  TESTABLE_ASSERT_EQUAL(io[5 + 1], values[1]);
  TESTABLE_ASSERT_EQUAL(io[5 + 2], values[2]);
  TESTABLE_ASSERT_EQUAL(io[5 + 3], values[3]);
  TESTABLE_ASSERT_EQUAL(io[5 + 4], values[4]);

  goto epilogue;
epilogue:
  return result;
}

DEFINE_TESTABLE(neural_net_weight_test)
{
  lsResult result = lsR_Success;

  constexpr size_t input_layer_block_count = 1;
  constexpr size_t output_layer_block_count = 2;
  neural_net<input_layer_block_count, output_layer_block_count> nn;
  decltype(nn)::io_buffer_t io;

  constexpr size_t expected_out_count = output_layer_block_count * neural_net_block_size;
  constexpr size_t actual_out_count = LS_ARRAYSIZE(io.data);
  static_assert(actual_out_count == expected_out_count);

  lsZeroMemory(&nn);
  lsZeroMemory(&io);

  constexpr int16_t expectedValue1 = 16;
  nn.data.weights[0] = 127;
  io[0] = expectedValue1;

  constexpr int16_t expectedValue2 = -128;
  nn.data.weights[input_layer_block_count * neural_net_block_size + 1] = 127;
  io[1] = expectedValue2;

  neural_net_eval(nn, io);

  const int16_t diff1 = lsAbs((int16_t)(expectedValue1 - io[0]));
  const int16_t diff2 = lsAbs((int16_t)(expectedValue2 - io[1]));

  TESTABLE_ASSERT_TRUE(diff1 <= 1);
  TESTABLE_ASSERT_TRUE(diff2 <= 1);

  goto epilogue;
epilogue:
  return result;
}

DEFINE_TESTABLE(neural_net_io_test)
{
  lsResult result = lsR_Success;

  constexpr size_t input_layer_block_count = 1;
  constexpr size_t output_layer_block_count = 2;
  neural_net<input_layer_block_count, output_layer_block_count> nn;
  lsCreateDirectory("_test");
  const char filename[] = "_test/nn_io_test";

  for (size_t i = 0; i < nn.total_value_count; i++)
    nn.values[i] = (int8_t)i;

  cached_file_byte_stream_writer<> write_stream;
  write_byte_stream_init(write_stream, filename);

  value_writer<decltype(write_stream)> writer;
  value_writer_init(writer, &write_stream);

  neural_net_write(nn, writer);
  write_byte_stream_flush(write_stream);

  neural_net<input_layer_block_count, output_layer_block_count> read_nn;
  lsZeroMemory(&read_nn);

  cached_file_byte_stream_reader<> read_stream;
  read_byte_stream_init(read_stream, filename);
  value_reader<cached_file_byte_stream_reader<>> reader;
  value_reader_init(reader, &read_stream);

  neural_net_read(read_nn, reader);
  read_byte_stream_destroy(read_stream);

  for (size_t i = 0; i < nn.total_value_count; i++)
    TESTABLE_ASSERT_EQUAL(nn.values[i], read_nn.values[i]);


  goto epilogue;
epilogue:
  return result;
}
