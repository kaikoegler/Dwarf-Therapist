/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "emotiongroup.h"
#include "dwarf.h"

void EmotionGroup::add_detail(Dwarf *d, UnitEmotion *ue){
    EMOTION_TYPE e_type = ue->get_emotion_type();
    int total_count = ue->get_count();
    if(m_details.contains(e_type)){
        m_details[e_type].count += total_count;
        m_details[e_type].unit_ids.insert(d->nice_name(),d->id());
    }else{
        emotion_count ec;
        ec.count = total_count;
        ec.unit_ids.insert(d->nice_name(),d->id());
        m_details.insert(e_type,ec);
    }

    if(ue->get_stress_effect() > 0){
        if(!m_stress_ids.contains(d->id()))
            m_stress_ids.append(d->id());
        m_stress_count += total_count;
    }else if(ue->get_stress_effect() < 0){
        if(!m_eustress_ids.contains(d->id()))
            m_eustress_ids.append(d->id());
        m_eustress_count += total_count;
    }else{
        if(!m_unaffected_ids.contains(d->id()))
            m_unaffected_ids.append(d->id());
        m_unaffected_count += total_count;
    }
}
