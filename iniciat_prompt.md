# Trying promtp engineering

In this first prompt, i'll try to explain the whole idea of the game, the mechanics, the variables, the player, etc. I'll try to be very specific and clear, so the AI can understand the whole idea and develop the game properly.

Well I've been studying C++ for 2 weeks, I'm not an expert, but I'm willing to learn. Now i'm currently reading "C++ for engineers and scientists" by Gary J. Bronson. I'm in the three chapter, it's about variables and constants. You will have to help me with the code, and explain me every part of the code, the concepts, the mechanics, etc. even if I don't ask you to explain me and if I don't even read that topic in the book. I want to learn while I develop the game.

* I want to build a game called Homo Politicus, it's a political simulator game where the player can control a political leader and try to manage the country, economy, international relations, etc. The game will be developed in C++ and will use SFML for graphics. First, we'll build the core of the game, then we'll add the graphics and the user interface.

* The core of the game will be a simulation of the political system, economy and international relations. First, we'll build a central brain of the game, it will be a class called Game. This class will have a method called update() that will be called every frame. This method will update the state of the game.

* Then, we'll build a class called Country. In the first step, we'll only work with one country. The player will be the leader of the country. The country will have a population, economy, law system, political system, etc. The external variables will be the international relations and the neighbors countries.

* These are the variables that the country will have, in this first step:

1. Welfare and Society System
Health: pandemic_prob, epidemic_prob, food_contamination_prob, obesity, health_coverage, food_radiation_prob, hospitals, products, mass_casualty_prob, severe_burn_prob, major_accident_prob.

Education: literacy, primary_enrollment, secondary_enrollment, university_enrollment, educational_quality, brain_drain, research_budget.

Labor and Social Security: unemployment_rate, union_strength, pension_sustainability, minimum_wage, labor_informality, general_strike_prob.

Demographics: birth_rate, death_rate, aging_index, urban_vs_rural_population, population_density.

Religion: clerical_political_influence, interreligious_tension, radicalism_prob, freedom_of_worship.

Human Rights: torture_index, forced_disappearances, freedom_of_expression, minority_protection, UN_score.

2. Economic and Financial System
Economy: GDP, remittances, tax_collection, inflation, growth_rate, international_reserves.

Central Bank: central_bank_autonomy, interest_rate, monetary_emission, exchange_rate_stability.

Sovereign Debt: debt_to_gdp_ratio, credit_rating (AAA-D), default_prob, debt_interest.

Natural Resources/Extraction: mining_concessions, state_royalties, community_conflicts, resource_depletion, commodity_prices.

Foreign Trade: trade_balance, average_tariffs, number_of_free_trade_agreements, import_dependency, international_sanctions_prob.

Tourism: annual_visitors, average_tourist_spending, tourist_safety, heritage_preservation, foreign_travel_alert_prob.

3. Political and Institutional System
Politics: popularity, stay_in_power_prob, authoritarianism_prob, coup_d_etat_prob, administrative_corruption.

Governance: marches, blockades, mobilizations, polarization_index.

Legislative: congressional_support, law_blockade_prob, party_fragmentation, lobbying_cost, pending_bills.

Governors: regional_loyalty, federal_budget_disparity, separatism_prob, provincial_autonomy.

Judiciary: judicial_independence, sentencing_speed, ruling_against_state_prob, trust_in_justice.

Prosecutor’s Office: number_of_prosecutors, number_of_case_files, total_offices, impunity.

Lobbies (Power Groups): industrial_sector_power, agricultural_sector_power, financial_sector_power, tech_sector_power.

4. Power, Security, and International Relations System
International Relations: proximity_to_superpower, mass_migration_prob, nuclear_attack_prob, invasion_prob, diplomatic_prestige.

Security: serial_killer_prob, weapons_in_population, non-state_groups, conflict_with_non-state_groups, homicide_rate.

Intelligence: espionage_budget, informant_network, document_leak_prob, cyber_surveillance, attack_detection_prob.

Defense: troop_morale, equipment_modernization, air_force, naval_force, military_insubordination_prob, military_spending_gdp.

Communication and Propaganda: media_control, reach_of_official_narrative, successful_fake_news_prob, press_freedom.

5. Infrastructure and Future System
Environment: pollution_prob, fauna_extinction_prob, drought_prob, storm_prob, tornado_prob, CO2_emissions.

Science and Technology: S&T_percentage_of_gdp, private_investment, patent_development, innovation_index.

Infrastructure: road_connectivity, port_capacity, airports, internet_coverage, access_to_potable_water, infrastructure_maintenance.

Energy: generation_capacity, fossil_fuel_dependency, renewables_percentage, oil_and_gas_reserves, mass_blackout_prob, kwh_price.

Space Race: satellite_capacity, space_budget, launch_failure_prob, technological_prestige.

Artificial Intelligence: state_ai_development, employment_automation, ai_cyberattack_prob, algorithmic_ethics.

* The last variables must be related with the others. For example, if the country has a high GDP, it can invest more in education, health, infrastructure, etc. If the country has a low GDP, it can't invest much in education, health, infrastructure, etc. If the country enters in recession, it can't invest much in education, health, infrastructure, etc. and it impacts in the leader popularity, approval, etc. The same logic applies to the other variables. The central brain will be in charge of updating the state of the game every frame. It will be a class called Game.

* The player: The player will be the leader of the country. The player will have a set of attributes that will determine his success or failure. Popularity is the main attribute, it will determine if the player stays in power or not. The player will have a set of actions that he can take to influence the state of the game. The player will have a set of attributes that will determine his success or failure. The player will have a set of actions that he can take to influence the state of the game.

* Try to manage some scenarios, for example: a leader conducts a coup d'etat, the country enters in recession, the country enters in war, the country enters in a pandemic, the country enters in a natural disaster, the country enters in a revolution, the country enters in a civil war, the country enters in a nuclear attack, the country enters in a invasion, the country enters in a diplomatic crisis, the country enters in a economic crisis, the country enters in a political crisis, the country enters in a social crisis, the country enters in a environmental crisis, the country enters in a technological crisis, the country enters in a space race crisis, the country enters in a artificial intelligence crisis, the country enters in a energy crisis, the country enters in a infrastructure crisis, the country enters in a labor and social security crisis, the country enters in a religion crisis, the country enters in a human rights crisis, the country enters in a international relations crisis, the country enters in a security crisis, the country enters in a intelligence crisis, the country enters in a defense crisis, the country enters in a communication and propaganda crisis, the country enters in a politics crisis, the country enters in a governance crisis, the country enters in a legislative crisis, the country enters in a governors crisis, the country enters in a judiciary crisis, the country enters in a prosecutor's office crisis, the country enters in a lobbies crisis, etc. These scenarios will be triggered by the player's actions or by random events. The player will have to make decisions to manage these scenarios and try to keep the country afloat. The player will have a set of attributes that will determine his success or failure. Popularity is the main attribute, it will determine if the player stays in power or not. The player will have a set of actions that he can take to influence the state of the game. The player will have a set of attributes that will determine his success or failure. The player will have a set of actions that he can take to influence the state of the game.

* Also, try to manage some internal attempts to manage with the pressure, example: the leader with low popularity is pressured by the congress to cede power, the judiciary will try to depose the leader by a law-fare, the party pressures the leader to cede power, the military pressures the leader to cede power, the people, other parties, lobbies, military, etc. pressures the leader to increase democracy, close the economy, open the economy, etc. These internal attempts will be triggered by the player's actions or by random events. The player will have to make decisions to manage these internal attempts and try to keep the country afloat. The player will have a set of attributes that will determine his success or failure. Popularity is the main attribute, it will determine if the player stays in power or not. The player will have a set of actions that he can take to influence the state of the game. The player will have a set of attributes that will determine his success or failure. The player will have a set of actions that he can take to influence the state of the game.

* Give life to the other actors, for example: the congress, the judiciary, the prosecutor's office, the lobbies, the military, the people, other parties, etc. Example, the military can conduct a coup d'etat, the judiciary can try to depose the leader by a law-fare, the party pressures the leader to cede power, the military pressures the leader to cede power, the people, other parties, lobbies, military, etc. pressures the leader to increase democracy, close the economy, open the economy, etc.

* Also, try to insert ideological spectrums with the mix of left and right, combined with the scenarios, for example: the leader is left and the country is in a economic crisis, the leader will try to implement left policies to solve the economic crisis, but the right will oppose to the leader's policies, if the leader is unpopular, the right will try to take advantage of the situation to gain power or ally with the army to conduct a coup d'etat, etc. The same logic applies to the other ideological spectrums. Another example: the leader is left-wing, won the last election with a large margin and he/she gains popularity, so the leader will try to copt the judiciary, the congress, the prosecutor's office, the lobbies, the military, the people, other parties, etc. to implement his policies. She/he will try to depose the judiciary, the congress, the prosecutor's office, the lobbies, the military, the people, other parties, etc. to implement his policies. The same logic applies to the other ideological spectrums.

* Also, try to implement an authoritarian-democratic spectrum, combined with the ideological spectrums, for example: for example, the leader is right-wind, authoritarian but umpopular, the country is in a economic crisis, so the left-wing gains the elections, but the right-wing leader don't recognize the elections and conducts a coup d'etat, etc. The same logic applies to the other ideological spectrums.

* Also, manage scandals, for example: the leader is involved in a corruption scandal, the leader is involved in a sex scandal, the leader is involved in a drug scandal, the leader is involved in a alcohol scandal, the leader is involved in a gambling scandal, the leader is involved in a insider trading scandal, the leader is involved in a tax evasion scandal, the leader is involved in a money laundering scandal, the leader is involved in a fraud scandal, the leader is involved in a embezzlement scandal, the leader is involved in a bribery scandal, the leader is involved in a extortion scandal, the leader is involved in a blackmail scandal, the leader is involved in a harassment scandal, the leader is involved in a assault scandal, the leader is involved in a domestic violence scandal, the leader is involved in a sexual assault scandal, the leader is involved in a rape scandal, the leader is involved in a murder scandal, the leader is involved in a treason scandal, the leader is involved in a espionage scandal, the leader is involved in a terrorism scandal, the leader is involved in a organized crime scandal, the leader is involved in a human trafficking scandal, the leader is involved in a arms trafficking scandal, the leader is involved in a drug trafficking scandal, the leader is involved in a human rights scandal, the leader is involved in a environmental scandal, the leader is involved in a technological scandal, the leader is involved in a space race scandal, the leader is involved in a artificial intelligence scandal, the leader is involved in a energy scandal, the leader is involved in a infrastructure scandal, the leader is involved in a labor and social security scandal, the leader is involved in a religion scandal, the leader is involved in a human rights scandal, the leader is involved in a international relations scandal, the leader is involved in a security scandal, the leader is involved in a intelligence scandal, the leader is involved in a defense scandal, the leader is involved in a communication and propaganda scandal, the leader is involved in a politics scandal, the leader is involved in a governance scandal, the leader is involved in a legislative scandal, the leader is involved in a governors scandal, the leader is involved in a judiciary scandal, the leader is involved in a prosecutor's office scandal, the leader is involved in a lobbies scandal, etc. These scandals will be triggered by the player's actions or by random events. The player will have to make decisions to manage these scandals and try to keep the country afloat. The player will have a set of attributes that will determine his success or failure. Popularity is the main attribute, it will determine if the player stays in power or not. The player will have a set of actions that he can take to influence the state of the game. The player will have a set of attributes that will determine his success or failure. The player will have a set of actions that he can take to influence the state of the game.

* Well, try to implement this and i'll learn with you!