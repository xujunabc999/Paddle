/* Copyright (c) 2016 PaddlePaddle Authors. All Rights Reserve.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "paddle/framework/operator.h"
#include "gtest/gtest.h"
#include "paddle/framework/op_registry.h"

namespace paddle {
namespace framework {

class OperatorTest : public OperatorBase {
 public:
  void InferShape(const std::shared_ptr<Scope>& scope) const override {}
  void Run(const std::shared_ptr<Scope>& scope,
           const platform::DeviceContext& dev_ctx) const override {
    float scale = GetAttr<float>("scale");
    ASSERT_NEAR(scale, 3.14, 1e-5);
    ASSERT_EQ(scope->GetVariable(inputs_[0]), nullptr);
    ASSERT_NE(scope->GetVariable(outputs_[0]), nullptr);
  }
};

class OperatorTestProtoAndCheckerMaker : public OpProtoAndCheckerMaker {
 public:
  OperatorTestProtoAndCheckerMaker(OpProto* proto, OpAttrChecker* op_checker)
      : OpProtoAndCheckerMaker(proto, op_checker) {
    AddInput("input", "input of test op");
    AddOutput("output", "output of test op");
    AddAttr<float>("scale", "scale of cosine op")
        .SetDefault(1.0)
        .LargerThan(0.0);
    AddType("test_operator");
    AddComment("This is test op");
  }
};

REGISTER_OP(OperatorTest, OperatorTestProtoAndCheckerMaker, test_operator)

TEST(OperatorBase, all) {
  OpDesc op_desc;
  op_desc.set_type("test_operator");
  *op_desc.mutable_inputs()->Add() = "IN1";
  *op_desc.mutable_outputs()->Add() = "OUT1";
  auto attr = op_desc.mutable_attrs()->Add();
  attr->set_name("scale");
  attr->set_type(paddle::framework::AttrType::FLOAT);
  float scale = 3.14;
  attr->set_f(scale);

  platform::CPUDeviceContext device_context;
  auto scope = std::make_shared<Scope>();

  OperatorBase* op = paddle::framework::OpRegistry::CreateOp(op_desc);
  ASSERT_EQ(op->GetAttr<float>("scale"), scale);
  scope->CreateVariable("OUT1");
  op->Run(scope, device_context);
  std::cout << op->DebugString() << std::endl;
  delete op;
}

}  // namespace framework
}  // namespace paddle