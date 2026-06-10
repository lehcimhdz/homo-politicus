#include "TestFramework.hpp"
#include "FeedbackBuilder.hpp"

TEST_CASE(feedback_basic_action_header_present) {
    FeedbackBuilder fb("SUBES IMPUESTOS");
    std::string out = fb.build();
    CHECK(out.find("SUBES IMPUESTOS") != std::string::npos);
}

TEST_CASE(feedback_delta_pct_formats) {
    FeedbackBuilder fb("TEST");
    fb.addDelta("Popularidad", 0.60, 0.55);
    std::string out = fb.build();
    CHECK(out.find("Popularidad") != std::string::npos);
    CHECK(out.find("60") != std::string::npos);
    CHECK(out.find("55") != std::string::npos);
    CHECK(out.find("-5") != std::string::npos);
}

TEST_CASE(feedback_absolute_delta_formats) {
    FeedbackBuilder fb("TEST");
    fb.addAbsoluteDelta("Recaudacion", 100000000.0, 110000000.0);
    std::string out = fb.build();
    CHECK(out.find("100M") != std::string::npos);
    CHECK(out.find("110M") != std::string::npos);
    CHECK(out.find("+$10M") != std::string::npos);
}

TEST_CASE(feedback_risk_below_threshold_filtered) {
    FeedbackBuilder fb("TEST");
    fb.addRisk("evento_chico", 0.01);
    fb.addRisk("evento_grande", 0.10);
    std::string out = fb.build();
    CHECK(out.find("evento_chico") == std::string::npos);
    CHECK(out.find("evento_grande") != std::string::npos);
}

TEST_CASE(feedback_prediction_present) {
    FeedbackBuilder fb("TEST");
    fb.addPrediction("la inflacion seguira subiendo");
    std::string out = fb.build();
    CHECK(out.find("Proximo turno") != std::string::npos);
    CHECK(out.find("inflacion") != std::string::npos);
}
