#include <string>

#include "server/config/network/mongo_proxy.h"

#include "test/mocks/server/mocks.h"
#include "test/test_common/utility.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::NiceMock;
using testing::_;

namespace Envoy {
namespace Server {
namespace Configuration {

TEST(MongoFilterConfigTest, MongoProxy) {
  std::string json_string = R"EOF(
  {
    "stat_prefix": "my_stat_prefix",
    "access_log" : "path/to/access/log"
  }
  )EOF";

  Json::ObjectSharedPtr json_config = Json::Factory::loadFromString(json_string);
  NiceMock<MockFactoryContext> context;
  MongoProxyFilterConfigFactory factory;
  EXPECT_EQ(NetworkFilterType::Both, factory.type());
  NetworkFilterFactoryCb cb = factory.createFilterFactory(*json_config, context);
  Network::MockConnection connection;
  EXPECT_CALL(connection, addFilter(_));
  cb(connection);
}

void handleInvalidConfiguration(const std::string& json_string) {
  Json::ObjectSharedPtr json_config = Json::Factory::loadFromString(json_string);
  NiceMock<MockFactoryContext> context;
  MongoProxyFilterConfigFactory factory;

  EXPECT_THROW(factory.createFilterFactory(*json_config, context), Json::Exception);
}

TEST(MongoFilterConfigTest, EmptyConfig) { handleInvalidConfiguration("{}"); }

TEST(MongoFilterConfigTest, InvalidExtraProperty) {
  std::string json_string = R"EOF(
  {
    "stat_prefix": "my_stat_prefix",
    "access_log" : "path/to/access/log",
    "test" : "a"
  }
  )EOF";

  handleInvalidConfiguration(json_string);
}

TEST(MongoFilterConfigTest, InvalidEmptyFault) {
  std::string json_string = R"EOF(
  {
    "stat_prefix": "my_stat_prefix",
    "fault" : {}
  }
  )EOF";

  handleInvalidConfiguration(json_string);
}

TEST(MongoFilterConfigTest, InvalidMissingPercentage) {
  std::string json_string = R"EOF(
  {
    "stat_prefix": "my_stat_prefix",
    "fault" : {
      "delay_ms": 1
    }
  }
  )EOF";

  handleInvalidConfiguration(json_string);
}

TEST(MongoFilterConfigTest, InvalidMissingMs) {
  std::string json_string = R"EOF(
  {
    "stat_prefix": "my_stat_prefix",
    "fault" : {
      "delay_percent": 1
    }
  }
  )EOF";

  handleInvalidConfiguration(json_string);
}

TEST(MongoFilterConfigTest, InvalidNegativeMs) {
  std::string json_string = R"EOF(
  {
    "stat_prefix": "my_stat_prefix",
    "fault" : {
      "delay_percent": 1,
      "duration_ms": -1
    }
  }
  )EOF";

  handleInvalidConfiguration(json_string);
}

TEST(MongoFilterConfigTest, InvalidPercent) {
  {
    std::string json_string = R"EOF(
  {
    "stat_prefix": "my_stat_prefix",
    "fault" : {
      "delay_percent": 101,
      "duration_ms": 1
    }
  }
  )EOF";

    handleInvalidConfiguration(json_string);
  }

  {
    std::string json_string = R"EOF(
  {
    "stat_prefix": "my_stat_prefix",
    "fault" : {
      "delay_percent": -1,
      "duration_ms": 1
    }
  }
  )EOF";

    handleInvalidConfiguration(json_string);
  }
}

TEST(MongoFilterConfigTest, CorrectFaultConfiguration) {
  std::string json_string = R"EOF(
  {
    "stat_prefix": "my_stat_prefix",
    "fault" : {
      "delay_percent": 1,
      "duration_ms": 1
    }
  }
  )EOF";

  Json::ObjectSharedPtr json_config = Json::Factory::loadFromString(json_string);
  NiceMock<MockFactoryContext> context;
  MongoProxyFilterConfigFactory factory;
  EXPECT_EQ(NetworkFilterType::Both, factory.type());
  NetworkFilterFactoryCb cb = factory.createFilterFactory(*json_config, context);
  Network::MockConnection connection;
  EXPECT_CALL(connection, addFilter(_));
  cb(connection);
}

} // namespace Configuration
} // namespace Server
} // namespace Envoy
