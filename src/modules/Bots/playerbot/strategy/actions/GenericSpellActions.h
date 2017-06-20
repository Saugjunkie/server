#pragma once

#include "../Action.h"
#include "../../PlayerbotAIConfig.h"
#include "PlayerbotAI.h"

#define BEGIN_SPELL_ACTION(clazz, name) \
class clazz : public CastSpellAction \
        { \
        public: \
        clazz(PlayerbotAI* ai) : CastSpellAction(ai, name) {} \


#define END_SPELL_ACTION() \
    };

#define BEGIN_DEBUFF_ACTION(clazz, name) \
class clazz : public CastDebuffSpellAction \
        { \
        public: \
        clazz(PlayerbotAI* ai) : CastDebuffSpellAction(ai, name) {} \

#define BEGIN_RANGED_SPELL_ACTION(clazz, name) \
class clazz : public CastSpellAction \
        { \
        public: \
        clazz(PlayerbotAI* ai) : CastSpellAction(ai, name) {} \

#define BEGIN_MELEE_SPELL_ACTION(clazz, name) \
class clazz : public CastMeleeSpellAction \
        { \
        public: \
        clazz(PlayerbotAI* ai) : CastMeleeSpellAction(ai, name) {} \


#define END_RANGED_SPELL_ACTION() \
    };


#define BEGIN_BUFF_ON_PARTY_ACTION(clazz, name) \
class clazz : public BuffOnPartyAction \
        { \
        public: \
        clazz(PlayerbotAI* ai) : BuffOnPartyAction(ai, name) {}


namespace ai
{
    class CastSpellAction : public Action
    {
    public:
        CastSpellAction(PlayerbotAI* ai, string spell) : Action(ai, spell),
			range(sPlayerbotAIConfig.spellDistance)
        {
            this->spell = spell;
        }

		virtual string GetTargetName() { return "current target"; };
        virtual bool Execute(Event event);
		Item * FindPoison() const;
		Item* FindConsumable(uint32 displayId) const;
		virtual bool isPossible();
		virtual bool isUseful();
        virtual ActionThreatType getThreatType() { return ACTION_THREAT_SINGLE; }

		virtual NextAction** getPrerequisites()
		{
			if (range > sPlayerbotAIConfig.spellDistance)
				return NULL;
			else if (range > ATTACK_DISTANCE)
				return NextAction::merge( NextAction::array(0, new NextAction("reach spell"), NULL), Action::getPrerequisites());
			else
				return NextAction::merge( NextAction::array(0, new NextAction("reach melee"), NULL), Action::getPrerequisites());
		}

    protected:
        string spell;
		float range;
    };

	//---------------------------------------------------------------------------------------------------------------------
	class CastAuraSpellAction : public CastSpellAction
	{
	public:
		CastAuraSpellAction(PlayerbotAI* ai, string spell) : CastSpellAction(ai, spell) {}

		virtual bool isUseful();
	};

    //---------------------------------------------------------------------------------------------------------------------
    class CastMeleeSpellAction : public CastSpellAction
    {
    public:
        CastMeleeSpellAction(PlayerbotAI* ai, string spell) : CastSpellAction(ai, spell) {
			range = ATTACK_DISTANCE;
		}
    };

    //---------------------------------------------------------------------------------------------------------------------
    class CastDebuffSpellAction : public CastAuraSpellAction
    {
    public:
        CastDebuffSpellAction(PlayerbotAI* ai, string spell) : CastAuraSpellAction(ai, spell) {}
    };

    class CastDebuffSpellOnAttackerAction : public CastAuraSpellAction
    {
    public:
        CastDebuffSpellOnAttackerAction(PlayerbotAI* ai, string spell) : CastAuraSpellAction(ai, spell) {}
        Value<Unit*>* GetTargetValue()
        {
            return context->GetValue<Unit*>("attacker without aura", spell);
        }
        virtual string getName() { return spell + " on attacker"; }
        virtual ActionThreatType getThreatType() { return ACTION_THREAT_AOE; }
    };

	class CastBuffSpellAction : public CastAuraSpellAction
	{
	public:
		CastBuffSpellAction(PlayerbotAI* ai, string spell) : CastAuraSpellAction(ai, spell)
		{
			range = sPlayerbotAIConfig.spellDistance;
		}

        virtual string GetTargetName() { return "self target"; }
	};

	class CastEnchantItemAction : public CastSpellAction
	{
	public:
	    CastEnchantItemAction(PlayerbotAI* ai, string spell) : CastSpellAction(ai, spell)
		{
			range = sPlayerbotAIConfig.spellDistance;
		}

        virtual bool isPossible();
        virtual string GetTargetName() { return "self target"; }
	};

    //---------------------------------------------------------------------------------------------------------------------

    class CastHealingSpellAction : public CastAuraSpellAction
    {
    public:
        CastHealingSpellAction(PlayerbotAI* ai, string spell, uint8 estAmount = 15.0f) : CastAuraSpellAction(ai, spell)
		{
            this->estAmount = estAmount;
			range = sPlayerbotAIConfig.spellDistance;
        }
		virtual string GetTargetName() { return "self target"; }
        virtual bool isUseful();
        virtual ActionThreatType getThreatType() { return ACTION_THREAT_AOE; }

    protected:
        uint8 estAmount;
    };

    class CastAoeHealSpellAction : public CastHealingSpellAction
    {
    public:
    	CastAoeHealSpellAction(PlayerbotAI* ai, string spell, uint8 estAmount = 15.0f) : CastHealingSpellAction(ai, spell, estAmount) {}
		virtual string GetTargetName() { return "party member to heal"; }
        virtual bool isUseful();
    };

	class CastCureSpellAction : public CastSpellAction
	{
	public:
		CastCureSpellAction(PlayerbotAI* ai, string spell) : CastSpellAction(ai, spell)
		{
			range = sPlayerbotAIConfig.spellDistance;
		}

		virtual string GetTargetName() { return "self target"; }
	};

	class PartyMemberActionNameSupport {
	public:
		PartyMemberActionNameSupport(string spell)
		{
			name = string(spell) + " on party";
		}

		virtual string getName() { return name; }

	private:
		string name;
	};

    class HealPartyMemberAction : public CastHealingSpellAction, public PartyMemberActionNameSupport
    {
    public:
        HealPartyMemberAction(PlayerbotAI* ai, string spell, uint8 estAmount = 15.0f) :
			CastHealingSpellAction(ai, spell, estAmount), PartyMemberActionNameSupport(spell) {}

		virtual string GetTargetName() { return "party member to heal"; }
		virtual string getName() { return PartyMemberActionNameSupport::getName(); }
    };

	class ResurrectPartyMemberAction : public CastSpellAction
	{
	public:
		ResurrectPartyMemberAction(PlayerbotAI* ai, string spell) : CastSpellAction(ai, spell) {}

		virtual string GetTargetName() { return "party member to resurrect"; }
	};
    //---------------------------------------------------------------------------------------------------------------------

    class CurePartyMemberAction : public CastSpellAction, public PartyMemberActionNameSupport
    {
    public:
        CurePartyMemberAction(PlayerbotAI* ai, string spell, uint32 dispelType) :
			CastSpellAction(ai, spell), PartyMemberActionNameSupport(spell)
        {
            this->dispelType = dispelType;
        }

		virtual Value<Unit*>* GetTargetValue();
		virtual string getName() { return PartyMemberActionNameSupport::getName(); }

    protected:
        uint32 dispelType;
    };

    //---------------------------------------------------------------------------------------------------------------------

    class BuffOnPartyAction : public CastBuffSpellAction, public PartyMemberActionNameSupport
    {
    public:
        BuffOnPartyAction(PlayerbotAI* ai, string spell) :
			CastBuffSpellAction(ai, spell), PartyMemberActionNameSupport(spell) {}
    public:
		virtual Value<Unit*>* GetTargetValue();
		virtual string getName() { return PartyMemberActionNameSupport::getName(); }
    };

    //---------------------------------------------------------------------------------------------------------------------

    class CastShootAction : public CastSpellAction
    {
    public:
        CastShootAction(PlayerbotAI* ai) : CastSpellAction(ai, "shoot") {}
        virtual ActionThreatType getThreatType() { return ACTION_THREAT_NONE; }
    };
	
	//heal

	class CastCannibalizeAction : public CastHealingSpellAction
	{
	public:
		CastCannibalizeAction(PlayerbotAI* ai) : CastHealingSpellAction(ai, "cannibalize") {}
	};

	class CastDesperatePrayerAction : public CastHealingSpellAction
	{
	public:
		CastDesperatePrayerAction(PlayerbotAI* ai) : CastHealingSpellAction(ai, "desperate prayer") {}
	};

	//buff

	class CastShadowmeldAction : public CastBuffSpellAction
	{
	public:
		CastShadowmeldAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "shadowmeld") {}
	};

	class CastElunesGraceAction : public CastBuffSpellAction
	{
	public:
		CastElunesGraceAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "elune's grace") {}
	};

	class CastBerserkingAction : public CastBuffSpellAction
	{
	public:
		CastBerserkingAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "berserking") {}
	};

	class CastBloodFuryAction : public CastBuffSpellAction
	{
	public:
		CastBloodFuryAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "blood fury") {}
	};

	class CastStoneformAction : public CastBuffSpellAction
	{
	public:
		CastStoneformAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "stoneform") {}
	};

	class CastShadowguardAction : public CastBuffSpellAction
	{
	public:
		CastShadowguardAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "shadowguard") {}
	};

	class CastFeedbackAction : public CastBuffSpellAction
	{
	public:
		CastFeedbackAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "feedback") {}
	};

	//buff party

	class CastFearWardAction : public BuffOnPartyAction {
	public:
		CastFearWardAction(PlayerbotAI* ai) : BuffOnPartyAction(ai, "fear ward") {}
	};

	//debuff

	class CastTouchOfWeaknessAction : public CastDebuffSpellAction
	{
	public:
		CastTouchOfWeaknessAction(PlayerbotAI* ai) : CastDebuffSpellAction(ai, "touch of weakness") {}
	};

	class CastDevouringPlagueAction : public CastDebuffSpellAction
	{
	public:
		CastDevouringPlagueAction(PlayerbotAI* ai) : CastDebuffSpellAction(ai, "devouring plague") {}
	};

	class CastHexOfWeaknessAction : public CastDebuffSpellAction
	{
	public:
		CastHexOfWeaknessAction(PlayerbotAI* ai) : CastDebuffSpellAction(ai, "hex of weakness") {}
	};

	//spell

	class CastStarshardsAction : public CastSpellAction
	{
	public:
		CastStarshardsAction(PlayerbotAI* ai) : CastSpellAction(ai, "starshards") {}
	};

	class CastWillOfTheForsakenAction : public CastSpellAction
	{
	public:
		CastWillOfTheForsakenAction(PlayerbotAI* ai) : CastSpellAction(ai, "will of the forsaken") {}
	};

	class CastWarStompAction : public CastSpellAction
	{
	public:
		CastWarStompAction(PlayerbotAI* ai) : CastSpellAction(ai, "war stomp") {}
	};

	class CastEscapeArtistAction : public CastSpellAction
	{
	public:
		CastEscapeArtistAction(PlayerbotAI* ai) : CastSpellAction(ai, "escape artist") {}
	};


    class CastSpellOnEnemyHealerAction : public CastSpellAction
    {
    public:
        CastSpellOnEnemyHealerAction(PlayerbotAI* ai, string spell) : CastSpellAction(ai, spell) {}
        Value<Unit*>* GetTargetValue()
        {
            return context->GetValue<Unit*>("enemy healer target", spell);
        }
        virtual string getName() { return spell + " on enemy healer"; }
    };

}
